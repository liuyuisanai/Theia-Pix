#pragma once

template <typename Device, int TIMEOUT_MS>
struct BlockingDevice
{
	Device & dev;
	BlockingDevice(Device & d) : dev(d) {}

	friend inline ssize_t
	fileno(BlockingDevice & self) { return fileno(self.dev); }

	friend inline ssize_t
	read(BlockingDevice & self, void * buf, const size_t size)
	{
		// TODO check total time.
		char * b = (char *) buf;
		size_t i;
		ssize_t r = read(self.dev, b, size);
		if (r == -1)
		{
			if (errno != EAGAIN and errno != EINTR) { return -1; }
			i = 0;
		}
		else { i = r; }

		while (i < size)
		{
			struct pollfd p;
			p.fd = fileno(self.dev);
			p.events = POLLIN;
			p.revents = 0;
			r = poll(&p, 1, TIMEOUT_MS);
			if (r == -1)
			{
				if (errno != EAGAIN) { return -1; }
				else { continue; }
			}
			if (r == 0)
			{
				// Timeout.
				errno = ETIMEDOUT;
				return -1;
			}

			r = read(self.dev, b + i, size - i);
			if (r != -1) { i += r; }
			else if (errno != EAGAIN and errno != EINTR) { return -1; }
		}

		return size;
	}

	friend inline ssize_t
	write(BlockingDevice & self, const void * buf, const size_t size)
	{
		// TODO check total time.
		const char * b = (const char *) buf;
		size_t i;
		ssize_t r = write(self.dev, b, size);
		if (r == -1)
		{
			if (errno != EAGAIN and errno != EINTR) { return -1; }
			i = 0;
		}
		else { i = r; }

		while (i < size)
		{
			struct pollfd p;
			p.fd = fileno(self.dev);
			p.events = POLLOUT;
			p.revents = 0;
			r = poll(&p, 1, TIMEOUT_MS);
			if (r == -1)
			{
				if (errno != EAGAIN) { return -1; }
				else { continue; }
			}
			if (r == 0)
			{
				// Timeout.
				errno = ETIMEDOUT;
				return -1;
			}

			r = write(self.dev, b + i, size - i);
			if (r != -1) { i += r; }
			else if (errno != EAGAIN and errno != EINTR) { return -1; }
		}

		return size;
	}
};

template < size_t TIMEOUT_MS, typename Device >
BlockingDevice< Device, TIMEOUT_MS >
make_it_blocking(Device & d)
{ return BlockingDevice< Device, TIMEOUT_MS >( d ); }

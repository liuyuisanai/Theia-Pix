#include <cstring>
#include <poll.h>

class Mode
{
    public:
        Mode(char* descr, struct pollfd btn) : btnfd(btn){
            if (strlen(descr) > 100) {
                DOG_PRINT("[error] description too long, truncating\n");
                strncpy(description, descr, 100);
            } else {
                strncpy(description, descr, strlen(descr));
            }
        };
        virtual ~Mode() {};
        virtual Mode* execute() = 0;
    protected:
        char name[50];
        char description[100];
        struct pollfd btnfd;
};

class ModeLandScreen : public Mode {
    public:
        ModeLandScreen(char* descr, struct pollfd btn) : Mode(descr, btn) {};
        virtual ~ModeLandScreen() {};
        virtual Mode* execute();
};

class ModeFlyScreen : public Mode {
    public:
        virtual ~ModeFlyScreen() {};
        virtual Mode* execute();
};

/* === separate screens classes === */
/*          WHILE LANDED            */

class Startup : public Mode {
    public:
        Startup(char* descr, struct pollfd btn) : Mode(descr, btn) {strcpy(name, "[startup] {logo screen}");};
        virtual ~Startup() {};
        virtual Mode* execute();
    private:
        bool not_paired;
        bool not_connected;
};

class Pairing : public Mode {
    public:
        Pairing(char* descr, struct pollfd btn) : Mode(descr, btn) {strcpy(name, "[startup] {pairing}");};
        virtual ~Pairing() {};
        virtual Mode* execute();
};

class GMainScreen : public ModeLandScreen {
    public:
        GMainScreen(char* descr, struct pollfd btn) : ModeLandScreen(descr, btn) {strcpy(name,"[landed] {main menu}");};
        virtual ~GMainScreen() {};
        virtual Mode* execute();
};

class NoConnect : public Mode {
    public:
        NoConnect(char* descr,struct pollfd btn, bool l_was_lost) : Mode(descr, btn){
            was_lost = l_was_lost;
            if (was_lost) {
                strcpy(name,"[error] {connection lost}");
            } else {
                strcpy(name,"[error] {never connected}");
            }
        };
        virtual ~NoConnect() {};
        virtual Mode* execute();
    private:
        bool was_lost;
};

#include <string>
#include <vector>

class Mode
{
    public:
        Mode(std::string descr) {description = descr;};
        virtual ~Mode() {};
        virtual Mode* execute() = 0;
    protected:
        std::string name;
        std::string description;
};

class ModeLandScreen : public Mode {
    public:
        ModeLandScreen(std::string descr) : Mode(descr) {};
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
        Startup(std::string descr) : Mode(descr) {name = "[startup] {logo screen}";};
        virtual ~Startup() {};
        virtual Mode* execute();
    private:
        bool not_paired;
        bool not_connected;
};

class Pairing : public Mode {
    public:
        Pairing(std::string descr) : Mode(descr) {name = "[startup] {pairing}";};
        virtual ~Pairing() {};
        virtual Mode* execute();
};

class GMainScreen : public ModeLandScreen {
    public:
        GMainScreen(std::string descr) : ModeLandScreen(descr) {name = "[landed] {main menu}";};
        virtual ~GMainScreen() {};
        virtual Mode* execute();
};

class NoConnect : public Mode {
    public:
        NoConnect(std::string descr, bool l_was_lost) : Mode(descr){
            was_lost = l_was_lost;
            if (was_lost) {
                name = "[error] {connection lost}";
            } else {
                name = "[error] {never connected}";
            }
        };
        virtual ~NoConnect() {};
        virtual Mode* execute();
    private:
        bool was_lost
};


#include <iostream>
#include <string>
#include <vector>
#include <cctype>

enum CommandCharacter : char {
    REGISTRER = 'R',
    LAG_OVERSIKT = 'L',
    PASIENT_OVERSIKT = 'P',
    MEDISIN_OVERSIKT = 'M',
    FJERN = 'F',
    UT_TIL_FIL = 'U',
    INN_FRA_FIL = 'I',
    HJELP = 'H',
    QUIT = 'Q'
};

struct Command {
    using ActionType = bool (*)(const Command&);

    const char character{};
    const ActionType action;  // @doc implicitly cast lamba to c-function pointer - https://stackoverflow.com/a/23616995 - 05.03.18
    const std::string textHelp{};
    const std::string textSuccess{};
    const std::string textExit{};
    const std::string textFailure{};

    Command() = default;
};

namespace Action {
auto registrerNyResept        = [](const Command&) -> bool { return true; };
auto lagOversiktReseptLege    = [](const Command&) -> bool { return true; };
auto lagOversiktReseptPasient = [](const Command&) -> bool { return true; };
auto lagOversiktMedisinBruk   = [](const Command&) -> bool { return true; };
auto fjernGamleResepter       = [](const Command&) -> bool { return true; };
auto skrivTilFil              = [](const Command&) -> bool { return true; };
auto lesInnFraFil             = [](const Command&) -> bool { return true; };
auto hjelp                    = [](const Command&) -> bool { return true; };
auto quit                     = [](const Command&) -> bool { return false; };
}

namespace jet {
constexpr auto printline = [](auto string) { 
    std::cout << string << '\n';
};
}

int main() {

    const std::vector<Command> validCommands = {
        {REGISTRER,        Action::registrerNyResept,        "Registrer en ny resept", "Ny resept registert", "Registrering av resept mislykket, venligst prøv igjen.."},
        {LAG_OVERSIKT,     Action::lagOversiktReseptLege,    "Lag oversikt over reseptene fra en bestemt lege", "Viser oversikt", "Ingenting å vise"}, 
        {PASIENT_OVERSIKT, Action::lagOversiktReseptPasient, "Lag oversikt over resepter til en bestemt pasient", "Viser oversikt", "Ingenting å vise"}, 
        {MEDISIN_OVERSIKT, Action::lagOversiktMedisinBruk,   "Lag oversikt over bruk av bestemte medisiner", "Viser oversikt", "Ingenting å vise"},
        {FJERN,            Action::fjernGamleResepter,       "Fjern alle gamle resepter", "Fjernet alle gamle resepter", "Fjernet ingen resepter"},
        {UT_TIL_FIL,       Action::skrivTilFil,              "Skriv ut til fil", "Skriving til fil gikk bra", "Skriving til fil var mislykket. Avslutter applikasjon..."},
        {INN_FRA_FIL,      Action::lesInnFraFil,             "Les inn fra fil", "Lesing fra fil gikk bra", "Lesing fra fil var mislykket. Avslutter applikasjon..."},
        {HJELP,            Action::hjelp,                    "Hjelp", "Skriver ut liste med kommandoer"},
        {QUIT,             Action::quit,                     "Quit",  "", "Quitting..."}
    };

    const auto readValidCommand = [&validCommands]() -> Command {
        
        auto printValidCommands = [&validCommands]() {
            jet::printline("\nCommand list:          ");
            jet::printline("-------------------------");
            for (const auto& c : validCommands) {
                jet::printline(" " + std::string(1, c.character) + " : " + c.textHelp);
            }
        };

        std::string input{};
        printValidCommands();
        for (;;) {

            std::getline(std::cin, input);
            // 1. Check if length is correct
            if (input.size() > 1 || input.size() < 1) {
                jet::printline("Bad command, too many characters...");
                continue;
            }
            // 2. Check if input is of one of the accepted characters
            for (const auto c : validCommands) {
                if (std::toupper(input[0]) == c.character)
                    return c;
            }
            jet::printline("Unknown command, try something else...");
        }
    };

    for(;;) {
        Command command = readValidCommand();

        jet::printline("Running " + command.textHelp);        
        
        if(command.action(command)) { 
            jet::printline(command.textSuccess);
        } else {        
            jet::printline(command.textExit);        
            break;
        }
    }

    return 0;
}
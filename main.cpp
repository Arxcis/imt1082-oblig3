#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cctype>
#include <cstdint>

using uint8  = std::uint8_t;
using uint32 = std::uint32_t;    
using uint64 = std::uint64_t;        

namespace jet {
template<class Arg>
constexpr void printline(Arg arg) {  
    std::cout << arg << '\n';    
}

template<class Arg, class ...Args>
constexpr void printline(Arg arg, Args ... args) { 
    std::cout << arg << ' ';
    printline(std::forward<Args>(args)...);  
}
} // JET END

struct Context {
    struct Resept {
        using string = std::string;
        string dato;
        string pasientNavn;
        string legeNavn;
        string legeAdresse;
        string legeTelefon;
        string medisinNavn;
        uint64 medisinMengde; // milligram, mg
    };
    std::vector<Resept> resepter{}; 
};

struct Command {
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

    enum ReturnCode : uint8 {
        SUCCESS, ABORT, EXIT
    };

    using ActionType = ReturnCode (*)(Context&);
    const char character{};
    const ActionType action;  // @doc implicitly cast lamba to c-function pointer - https://stackoverflow.com/a/23616995 - 05.03.18
    const std::string textHelp{};
    const std::string textSuccess{};
    const std::string textExit{};
    const std::string textAbort{};
};


namespace Action {
auto registrerNyResept(Context& ctx) -> Command::ReturnCode { 

    constexpr auto getline = [](const auto msg, std::string& line){
        std::cout << msg << " : ";
        std::getline(std::cin, line);
    };

    constexpr auto getnumber = [](const auto msg, auto& number){
        std::cout << msg << " : ";
        std::cin >> number;
    };

    Context::Resept resept;
    getline("Dato (ÅÅÅÅMMDD)",               resept.dato);
    getline("Pasient navn  (Jonas Solsvik)", resept.pasientNavn);
    getline("Lege navn     (-------------)", resept.legeNavn);
    getline("Lege addresse (Snorres veg)  ", resept.legeAdresse);
    getline("Lege telefon  (45 20 08 77)  ", resept.legeTelefon);
    getline("Medisin navn               ",   resept.medisinNavn);
    getnumber("Medisin mengde i milligram",  resept.medisinMengde);
    
    ctx.resepter.push_back(resept);
    return Command::SUCCESS; 
};


auto lagOversiktReseptLege(Context& ctx) -> Command::ReturnCode { return Command::SUCCESS; };
auto lagOversiktReseptPasient(Context& ctx) -> Command::ReturnCode { return Command::SUCCESS; };
auto lagOversiktMedisinBruk(Context& ctx) -> Command::ReturnCode { return Command::SUCCESS; };
auto fjernGamleResepter(Context& ctx) -> Command::ReturnCode { return Command::SUCCESS; };

auto lesFraFil(Context& ctx) -> Command::ReturnCode
{ 
    std::ifstream innfil("../resept.dta", std::ios::binary);                                         
    if (!innfil) {
        return Command::EXIT;
    }    

    innfil.seekg (0, innfil.end);

    uint32 length = innfil.tellg();
    uint32 reseptCount = length/sizeof(Context::Resept);

    // Is file empty
    if (reseptCount == 0) {
        return Command::ABORT;
    }

    jet::printline("Fant", reseptCount, "resepter på fil!");
    innfil.seekg (0, innfil.beg);
    ctx.resepter.resize(reseptCount);
    innfil.read((char*) ctx.resepter.data(), sizeof(Context::Resept) * ctx.resepter.size());

    return Command::SUCCESS;
}


auto skrivTilFil(Context& ctx) -> Command::ReturnCode { 
    std::ofstream utfil("../resept.dta", std::ios::binary);
    if (!utfil) {
        return Command::EXIT;
    }  
    utfil.write((char*) ctx.resepter.data(), sizeof(Context::Resept) * ctx.resepter.size());
    return Command::SUCCESS;
}

auto hjelp(Context& ctx) -> Command::ReturnCode { return Command::SUCCESS; };
auto quit(Context& ctx) -> Command::ReturnCode { return Command::EXIT; };
} // ACTION END




int main() {
    Context ctx{};
    const std::vector<Command> validCommands = {
        {Command::REGISTRER, Action::registrerNyResept,
            "Registrer en ny resept",                                   // help text
            "Ny resept registert",                                      // success text
            "",                                                         // exit text
            "Registrering av resept mislykket, venligst prøv igjen.."}, // success abort

        {Command::LAG_OVERSIKT, Action::lagOversiktReseptLege,    
            "Lag oversikt over reseptene fra en bestemt lege", 
            "", 
            "",                                                  
            "Ingen resepter på denne legen"}, 

        {Command::PASIENT_OVERSIKT, Action::lagOversiktReseptPasient, 
            "Lag oversikt over resepter til en bestemt pasient", 
            "",
            "",                           
            "Ingen resepter på denne pasienten"}, 

        {Command::MEDISIN_OVERSIKT, Action::lagOversiktMedisinBruk,   
            "Lag oversikt over bruk av bestemte medisiner", 
            "",
            "",
            "Ingen resepter finnes i databasen"},

        {Command::FJERN, Action::fjernGamleResepter,       
            "Fjern alle gamle resepter", 
            "Fjernet alle gamle resepter",
            "",
            "All resepter er allerede fjernet"},

        {Command::UT_TIL_FIL, Action::skrivTilFil, 
            "Skriv ut til fil", 
            "Skriving til fil gikk bra", 
            "Skriving til fil var mislykket. Avslutter applikasjon...",
            ""},

        {Command::INN_FRA_FIL, Action::lesFraFil, 
            "Les inn fra fil", 
            "Lesing fra fil gikk bra", 
            "Lesing fra fil var mislykket. Avslutter applikasjon...", 
            "Ingenting på fil, avbryter lesing fra fil.."},

        {Command::HJELP, Action::hjelp, 
            "Hjelp", 
            "Skriver ut liste med kommandoer",
            "",
            ""},

        {Command::QUIT, Action::quit, 
            "Quit",  
            "", 
            "Quitting...",
            ""}
    };

    Action::lesFraFil(ctx);
    for(;;) {

        constexpr auto readValidCommand = [](const auto& validCommands) -> Command {
        
            constexpr auto printValidCommands = [](const auto& validCommands) {
                jet::printline("\nCommand list:          ");
                jet::printline("--------------------------------------------");
                for (const auto& c : validCommands) {
                    jet::printline(" ", c.character, ":", c.textHelp);
                }
            };

            std::string input{};
            printValidCommands(validCommands);
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

        auto command = readValidCommand(validCommands);
        jet::printline("Running", command.textHelp);        
        
        auto status = command.action(ctx);

        // Exit program
        if (status == Command::EXIT) { 
            jet::printline(command.textExit);
            break;            
        } else
        // Abort current command, and ask for new command         
        if (status == Command::ABORT) {
            jet::printline(command.textAbort);
            continue;
        }    
        jet::printline(command.textSuccess);        
    
    } // END FOR
    
    Action::skrivTilFil(ctx);
    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cctype>
#include <cstdint>


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
}

enum ReturnCode : int {
     SUCCESS, ABORT, EXIT
};

struct Resept {
    using string = std::string;
    using uint32 = std::uint32_t;
    string dato;
    string pasientNavn;
    string legeNavn;
    string legeAdresse;
    string legeTelefon;
    string medisinNavn;
    uint32 medisinMengde;
};

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

struct Context {
    std::vector<Resept> resepter{}; 
};
struct Command {
    using ActionType = ReturnCode (*)(Context&);

    const char character{};
    const ActionType action;  // @doc implicitly cast lamba to c-function pointer - https://stackoverflow.com/a/23616995 - 05.03.18
    const std::string textHelp{};
    const std::string textSuccess{};
    const std::string textExit{};
    const std::string textAbort{};
};


namespace Action {
auto registrerNyResept(Context& ctx) -> ReturnCode { 

    constexpr auto getline = [](const auto msg, std::string& line){
        std::cout << msg << " : ";
        std::getline(std::cin, line);
    };

    constexpr auto getnumber = [](const auto msg, auto& number){
        std::cout << msg << " : ";
        std::cin >> number;
    };

    Resept resept;
    getline("Dato (ÅÅÅÅMMDD)", resept.dato);
    getline("Pasient navn  (Jonas Solsvik)", resept.pasientNavn);
    getline("Lege navn     (-------------)", resept.legeNavn);
    getline("Lege addresse (Snorres veg)  ", resept.legeAdresse);
    getline("Lege telefon  (45 20 08 77)  ", resept.legeTelefon);
    getline("Medisin navn               ",   resept.medisinNavn);
    getnumber("Medisin mengde i milligram",  resept.medisinMengde);
    
    ctx.resepter.push_back(resept);
    return SUCCESS; 
};


auto lagOversiktReseptLege(Context& ctx) -> ReturnCode { return SUCCESS; };
auto lagOversiktReseptPasient(Context& ctx) -> ReturnCode { return SUCCESS; };
auto lagOversiktMedisinBruk(Context& ctx) -> ReturnCode { return SUCCESS; };
auto fjernGamleResepter(Context& ctx) -> ReturnCode { return SUCCESS; };

auto lesFraFil(Context& ctx) -> ReturnCode
{ 
    std::ifstream innfil("../resept.dta", std::ios::binary);                                         
    if (!innfil) {
        return EXIT;
    }    

    innfil.seekg (0, innfil.end);
    std::size_t length = innfil.tellg();
    std::size_t reseptCount = length/sizeof(Resept);
    // Nothing to load from file
    if (length == 0) {
        return ABORT;
    }

    jet::printline("Fant", reseptCount, "resepter på fil!");
    innfil.seekg (0, innfil.beg);
    ctx.resepter.resize(reseptCount);
    innfil.read((char*) ctx.resepter.data(), sizeof(Resept) * ctx.resepter.size());

    return SUCCESS;
}


auto skrivTilFil(Context& ctx) -> ReturnCode { 
    std::ofstream utfil("../resept.dta", std::ios::binary);
    if (!utfil) {
        return EXIT;
    }  
    utfil.write((char*) ctx.resepter.data(), sizeof(Resept) * ctx.resepter.size());
    return SUCCESS;
}

auto hjelp(Context& ctx) -> ReturnCode { return SUCCESS; };
auto quit(Context& ctx) -> ReturnCode { return EXIT; };
}




int main() {

    Context ctx{};
    const std::vector<Command> validCommands = {
        {REGISTRER, Action::registrerNyResept,
                    "Registrer en ny resept", 
                    "Ny resept registert",
                    "",
                    "Registrering av resept mislykket, venligst prøv igjen.."},

        {LAG_OVERSIKT, Action::lagOversiktReseptLege,    
                       "Lag oversikt over reseptene fra en bestemt lege", 
                       "", 
                       "",                                                  
                       "Ingen resepter på denne legen"}, 

        {PASIENT_OVERSIKT, Action::lagOversiktReseptPasient, 
                           "Lag oversikt over resepter til en bestemt pasient", 
                           "",
                           "",                           
                           "Ingen resepter på denne pasienten"}, 

        {MEDISIN_OVERSIKT, Action::lagOversiktMedisinBruk,   
                           "Lag oversikt over bruk av bestemte medisiner", 
                           "",
                           "",
                           "Ingen resepter finnes i databasen"},

        {FJERN, Action::fjernGamleResepter,       
                "Fjern alle gamle resepter", 
                "Fjernet alle gamle resepter",
                "",
                "All resepter er allerede fjernet"},

        {UT_TIL_FIL, Action::skrivTilFil, 
                     "Skriv ut til fil", 
                     "Skriving til fil gikk bra", 
                     "Skriving til fil var mislykket. Avslutter applikasjon..."},

        {INN_FRA_FIL, Action::lesFraFil, 
                      "Les inn fra fil", 
                      "Lesing fra fil gikk bra", 
                      "Lesing fra fil var mislykket. Avslutter applikasjon...", 
                      "Ingenting på fil, avbryter lesing fra fil.."},

        {HJELP, Action::hjelp, 
                "Hjelp", 
                "Skriver ut liste med kommandoer"},

        {QUIT, Action::quit, 
                "Quit",  
                "", 
                "Quitting..."}
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
        if (status == EXIT) { 
            jet::printline(command.textExit);
            break;            
        } else 
        if (status == ABORT) {
            jet::printline(command.textAbort);
            continue;
        }    
        jet::printline(command.textSuccess);        
    }
    Action::skrivTilFil(ctx);
    return 0;
}
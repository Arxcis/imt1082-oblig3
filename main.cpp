#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cctype>
#include <cstdint>

#include "ListTool2a.h"

using uint8  = std::uint8_t;
using uint32 = std::uint32_t;    
using uint64 = std::uint64_t;        

const char* DATAFILE = "./resept.dta";

//
// @namespace JET - Jonas epic templates
//
namespace JET { 

//
// @class Listx
// @brief A class which wraps the ListTool class by inheriting from it. 
//         I am doing this for 2 reasons:
//         Reason1: I want to delete the default constructor to make it super clear
//                  (with compiler error) that the default constructor is not supposed to be used.
//         Reason2: I want to implement the std::forward_iterator concept, which makes it possible
//                  to use the list container with C++ 11 range-for loops.
//         Bonus:   By implementing std::forward_iterator I also get to use the list with many
//                  standard <algorithm> functions such as: std::count_if, std::partition,
//                  std::binary_search, std::find_if and many more.
//
template<class ElementType>
class Listx : public List
{
public:
    static_assert(std::is_base_of<Element, ElementType>::value, 
        "ElementType must inherit from Element, NumElement or TextElement");

    // @doc - https://stackoverflow.com/a/28926968/6292603 - 06.03.18
    struct iterator {
        Node* node;
        auto operator++() -> iterator&                { node = node->next;  return *this;  }
        auto operator++(int) -> iterator              { node = node->next;  return *this;  }
        bool operator!=(const iterator& other) const  { return node != other.node; }
        bool operator==(const iterator& other) const  { return node == other.node;  }
        auto operator*() -> ElementType&              { return *(ElementType*)node->listElement; }
        auto operator*() const -> const ElementType&  { return *(ElementType*)node->listElement; }
        auto operator->() -> ElementType*             { return (ElementType*)node->listElement;  }
        auto operator->() const -> const ElementType* { return (ElementType*)node->listElement;  }
    };
    Listx() = delete;
    Listx(ListType li): List(li) { std::cout << "Created ListEx\n"; }
    
    auto begin() const -> iterator { return iterator{ this->first }; }
    auto end()   const -> iterator { return iterator{ this->last }; }
};

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
    struct Resept : TextElement {

        using string = std::string;
        using uint64 = std::uint64_t;        
        
        string dato;
        string pasientNavn;
        string legeNavn;
        string legeAdresse;
        string legeTelefon;
        string medisinNavn;
        uint64 medisinMengde; // milligram, mg

        Resept(): TextElement("default") {}
        Resept(string _dato,
               string _pasientNavn,
               string _legeNavn,
               string _legeAdresse,
               string _legeTelefon,
               string _medisinNavn,
               uint64 _medisinMengde)
        :TextElement(_pasientNavn.c_str())
        ,dato(_dato)
        ,pasientNavn(_pasientNavn)
        ,legeNavn(_legeNavn)
        ,legeAdresse(_legeAdresse)
        ,legeTelefon(_legeTelefon)
        ,medisinNavn(_medisinNavn)
        ,medisinMengde(_medisinMengde)
        {}
    };
    JET::Listx<Resept> resepter{Sorted}; 
};

struct Command {
    enum CommandCharacter : char {
        iREGISTRER,
        iLAG_OVERSIKT,
        iPASIENT_OVERSIKT,
        iMEDISIN_OVERSIKT,
        iFJERN,
        iUT_TIL_FIL,
        iINN_FRA_FIL,
        iHJELP,
        iQUIT,
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
        CONTINUE, EXIT
    };
    using ActionType = ReturnCode (*)(Command&, Context&);    
    using string = std::string;    

    char character{};
    ActionType  action;
    string textHelp{};
    string textStatus{};

    auto status(ReturnCode code, const char* statusMsg) -> ReturnCode {
        textStatus = statusMsg;
        return code;
    }
};


namespace Action {
auto registrerNyResept(Command& cmd, Context& ctx) -> Command::ReturnCode { 

    constexpr auto getline = [](const auto msg) -> std::string {
        std::string line{};
        std::cout << msg << " : ";
        std::getline(std::cin, line);
        return line;
    };

    constexpr auto getnumber = [](const auto msg) -> uint64 {
        uint64 number;
        std::cout << msg << " : ";
        std::cin >> number;
        return number;
    };

    auto resept = new Context::Resept {
        getline("Dato (ÅÅÅÅMMDD)"),
        getline("Pasient navn  (Jonas Solsvik)"),
        getline("Lege navn     (-------------)"),
        getline("Lege addresse (Snorres veg)  "),
        getline("Lege telefon  (45 20 08 77)  "),
        getline("Medisin navn                 "),
        getnumber("Medisin mengde i milligram")   
    };

    ctx.resepter.add(resept);
    return cmd.status(Command::CONTINUE, "New resept added"); 
};


auto lagOversiktReseptLege(Command& cmd, Context& ctx) -> Command::ReturnCode { return Command::CONTINUE; };
auto lagOversiktReseptPasient(Command& cmd, Context& ctx) -> Command::ReturnCode { return Command::CONTINUE; };
auto lagOversiktMedisinBruk(Command& cmd, Context& ctx) -> Command::ReturnCode { return Command::CONTINUE; };
auto fjernGamleResepter(Command& cmd, Context& ctx) -> Command::ReturnCode { return Command::CONTINUE; };

auto lesFraFil(Command& cmd, Context& ctx) -> Command::ReturnCode
{ 
    std::ifstream innfil(DATAFILE, std::ios::binary);                                         
    if (!innfil) {
        return cmd.status(Command::EXIT, "Datafile not exist. Could not read. Please create resept.dta file and restart the app. Exiting!");
    }

    innfil.seekg (0, innfil.end);

    uint32 length = innfil.tellg();
    uint32 reseptCount = length/sizeof(*(ctx.resepter.begin()));

    // Is file empty
    if (reseptCount == 0) {
        return cmd.status(Command::CONTINUE, "The file innfile is empty. No resepts fround.");
    }

    JET::printline("Fant", reseptCount, "resepter på fil!");
    innfil.seekg (0, innfil.beg);
    
    for (uint32 i = 0; i < reseptCount; ++i) {
        auto resept = new Context::Resept{};
        innfil.read((char*) resept, sizeof(Context::Resept));
        ctx.resepter.add(resept);
    } 

    return cmd.status(Command::CONTINUE, "Finished reading file.");
}


auto skrivTilFil(Command& cmd, Context& ctx) -> Command::ReturnCode { 
    std::ofstream utfil(DATAFILE, std::ios::binary);
    if (!utfil) {
        return cmd.status(Command::EXIT, "Datafile not exist. Could not write. Please create file resept.dta and restart the app. EXITING!");
    }  
    
    for(const auto& resept : ctx.resepter) {
        utfil.write((char*) &resept, sizeof(Context::Resept));
    }
    return cmd.status(Command::CONTINUE, "Finished writing to file");
}

auto hjelp(Command& cmd, Context& ctx) -> Command::ReturnCode { return cmd.status(Command::CONTINUE, "Showing valid commands"); };
auto quit(Command& cmd, Context& ctx) -> Command::ReturnCode { return cmd.status(Command::EXIT, "Quitting app..."); };
} // ACTION END



int main() {
    Context ctx{};
    const std::vector<Command> validCommands = {
        {Command::REGISTRER, Action::registrerNyResept, 
            "Registrer en ny resept"},

        {Command::LAG_OVERSIKT, Action::lagOversiktReseptLege,    
            "Lag oversikt over reseptene fra en bestemt lege" },

        {Command::PASIENT_OVERSIKT, Action::lagOversiktReseptPasient, 
            "Lag oversikt over resepter til en bestemt pasient"}, 

        {Command::MEDISIN_OVERSIKT, Action::lagOversiktMedisinBruk,   
            "Lag oversikt over bruk av bestemte medisiner"},

        {Command::FJERN, Action::fjernGamleResepter,       
            "Fjern alle gamle resepter"},

        {Command::UT_TIL_FIL, Action::skrivTilFil, 
            "Skriv ut til fil"},

        {Command::INN_FRA_FIL, Action::lesFraFil, 
            "Les inn fra fil"},

        {Command::HJELP, Action::hjelp, 
            "Hjelp"},

        {Command::QUIT, Action::quit, 
            "Quit"}
    };

    {   // Load datastructure from file
        auto cmd = validCommands[Command::iINN_FRA_FIL];
        auto status = cmd.action(cmd, ctx);
        JET::printline(cmd.textStatus);
        if(status == Command::EXIT)
            return 0;
    }
    for(;;) {

        constexpr auto readValidCommand = [](const auto& validCommands) -> Command {
        
            constexpr auto printValidCommands = [](const auto& validCommands) {
                JET::printline("\nCommand list:                             ");
                JET::printline("--------------------------------------------");
                for (const auto& c : validCommands) {
                    JET::printline(" ", c.character, ":", c.textHelp);
                }
            };

            std::string input{};
            printValidCommands(validCommands);
            for (;;) {
                std::getline(std::cin, input);
                // 1. Check if length is correct
                if (input.size() > 1 || input.size() < 1) {
                    JET::printline("Bad command, too many characters...");
                    continue;
                }
                // 2. Check if input is of one of the accepted characters
                for (const auto& c : validCommands) {
                    if (std::toupper(input[0]) == c.character)
                        return c;
                }
                JET::printline("Unknown command, try something else...");
            }
        };

        auto command = readValidCommand(validCommands);
        JET::printline("Running", command.textHelp);        
        
        auto status = command.action(command, ctx);
        JET::printline(command.textStatus);

        if (status == Command::EXIT)
            break;
    
    }

    { // Dump datastructure to file
        auto cmd = validCommands[Command::iUT_TIL_FIL];
        cmd.action(cmd, ctx);
        JET::printline(cmd.textStatus);
    }
    return 0;
}
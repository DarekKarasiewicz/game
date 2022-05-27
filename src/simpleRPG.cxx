#include <iostream>
#include <map>
#include <string>
#include <vector>

enum class Profesion {
    Archer,
    Warrior,
    Rogue,
    Mag,
};


enum class Race {
    Human,
    Elf,
    Dwarf,
    Wolf,
    Troll,
    Dragon,
};


struct Character {
    int lv;
    std::string name;
    Profesion profesion;
    Race race;
};


auto give_me_int() -> int
{
    auto tmp = std::string{};
    std::getline(std::cin, tmp);
    return std::stoi(tmp);
}

auto hero_init() -> Character
{
    Character new_hero;
    std::cout << "Chose your name\n";
    /* std::cin >> new_hero.name ; */

    {
        std::cout << "Chose your Race\n";
        auto const races = std::map<Race, std::string>{
            {Race::Human, "Human"},
            {Race::Elf, "Elf"},
            {Race::Dwarf, "Dwarf"},
        };
        for (auto const [r, name] : races) {
            std::cout << static_cast<int>(r) << ": " << name << std::endl;
        }

        // Can I do the same think in switch
        /* Race user_choice_race =static_cast<Race>(give_me_int()); */
        /* user_choice_race */
        switch (auto const r = static_cast<Race>(give_me_int()); r) {
        case Race::Human:
        case Race::Elf:
        case Race::Dwarf:
            new_hero.race = r;
            break;
        default:
            std::cout << "Error\n";
        }
    }

    {
        std::cout << "Chose your Profesion\n";
        auto const profesions = std::map<Profesion, std::string>{
            {Profesion::Archer, "Archer"},
            {Profesion::Warrior, "Warrior"},
            {Profesion::Rogue, "Rogue"},
            {Profesion::Mag, "Mag"},
        };
        for (auto const [p, name] : profesions) {
            std::cout << static_cast<int>(p) << ": " << name << std::endl;
        }

        // Can I do the same think in switch?
        Profesion user_choice_profieson = static_cast<Profesion>(give_me_int());

        switch (user_choice_profieson) {
        case Profesion::Archer:
        case Profesion::Warrior:
        case Profesion::Rogue:
        case Profesion::Mag:
            new_hero.profesion = user_choice_profieson;
            break;
        default:
            std::cout << "Error\n";
        }
    }
    return new_hero;
}

auto enemy_init() -> Character
{
    Character new_enemy;
    {
        auto const races = std::map<Race, std::string>{
            {Race::Dragon, "Dragon"},
            {Race::Troll, "Troll"},
            {Race::Wolf, "Wolf"},
        };

        auto random = rand() % races.max_size();

        for (auto const [r, name] : races) {
            auto race_nr = static_cast<int>(r);
            if ((int)random == race_nr) {
                // Can I do the same think in switch
                auto pc_choice = static_cast<Race>(race_nr);

                switch (pc_choice) {
                case Race::Dragon:
                case Race::Troll:
                case Race::Wolf:
                    new_enemy.race = pc_choice;
                    break;
                default:
                    std::cout << "Error\n";
                }
            }
        }
    }

    {
        std::cout << "Chose your Profesion\n";
        auto const profesions = std::map<Profesion, std::string>{
            {Profesion::Archer, "Archer"},
            {Profesion::Warrior, "Warrior"},
            {Profesion::Rogue, "Rogue"},
            {Profesion::Mag, "Mag"},
        };

        auto random = rand() % profesions.max_size();
        for (auto const [p, name] : profesions) {
            auto profesion_nr = static_cast<int>(p);
            if ((int)random == profesion_nr) {
                auto pc_choice = static_cast<Profesion>(profesion_nr);

                switch (pc_choice) {
                case Profesion::Archer:
                case Profesion::Warrior:
                case Profesion::Rogue:
                case Profesion::Mag:
                    new_enemy.profesion = pc_choice;
                    break;
                default:
                    std::cout << "Error\n";
                }
            }
        }
    }
    return new_enemy;
}

auto to_string(Race v)
{
    switch (v) {
    case Race::Human:
        return "Human";
    case Race::Elf:
        return "Elf";
    case Race::Dwarf:
        return "Dwarf";
    default:
        return "Not found";
    }
}

auto to_string(Profesion v)
{
    switch (v) {
    case Profesion::Archer:
        return "Archer";
    case Profesion::Warrior:
        return "Warrior";
    case Profesion::Rogue:
        return "Rogue";
    case Profesion::Mag:
        return "Mag";
    default:
        return "Not found";
    }
}

auto main() -> int
{
    auto my_hero = hero_init();
    std::cout << my_hero.name << "\n";
    std::cout << to_string(my_hero.race) << "\n";
    std::cout << to_string(my_hero.profesion) << "\n";
    return 0;
}

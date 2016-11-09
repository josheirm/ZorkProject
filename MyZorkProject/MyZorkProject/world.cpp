#include <iostream>
#include <string>
#include <fstream>
#include "world.h"
#include "json11.hpp"

using namespace json11;

World::World()
{
	string jsonContentString;
	ifstream myfile("json_data/rooms.json");
	if (myfile.is_open())
	{
		string line;
		while (myfile.good())
		{
			getline(myfile, line);
			jsonContentString.append(line);
			//cout << line << endl;
		}
		myfile.close();
	}

	string err;
	Json jsonObj = Json::parse(jsonContentString, err);
	list<Room*> rooms;
	for (auto item : jsonObj.array_items()) {
		Room* room = new Room("", "");
		for (auto propertyValue : item.object_items()) {
			if (propertyValue.first == "name") {
				room->name = propertyValue.second.string_value();
			}
			if (propertyValue.first == "description") {
				room->description = propertyValue.second.string_value();
			}
		}
		printMessage(room->name, room->description);
	}



	commands = {
		new Command({ "look", "l" }, LOOK, 1),
		new Command({ "go", "g" }, GO, 1),
		new Command({ "take", "pick", "get", "t" }, TAKE, 1),
		new Command({ "drop", "d" }, DROP, 1),
		new Command({ "craft", "c" }, CRAFT, 1),
		new Command({ "eat", "e" }, EAT, 1),
		new Command({ "inventory", "i" }, INVENTORY, 1),
		new Command({ "quit", "q", "exit" }, QUIT, 1),
	};
	
	Room* cliff = new Room("Base of the cliff"s, "You're in the north wall of a big mountain. You can see three paths, one on each direction. In the wall behind you there's a painting."s);
	Room* cave = new Room("Cave"s, "You're inside a small and cold cave. It's a dead end."s);
	Room* forest = new Room("Forest"s, "This is a dense forest. There's a high tree with some branches you can reach."s);
	Room* upTheTree = new Room("Up the tree"s, "After climbing over 5 meters you've reached the top of the tree. From here you can see your camp at the far north."s);
	Room* clearing = new Room("Clearing"s, "You're in a clearing with a forest surrounding you in all directions but east."s);
	Room* wellEntrance = new Room("Well entrance"s, "You're in a clearing with a natural well big enough for a person to go through."s);
	Room* downTheWell = new Room("Inside the well"s, "It's dark and cold in here. A dead and decayed animal rests on the floor. It probably died from the fall."s);
	Room* path = new Room("Narrow path"s, "It's a very tight path between two mountain walls. You can see ahead some creature's lair, it looks dangerous!"s);
	Room* end = new Room("Smilodon's lair"s, "You face Smilodon: The Saber-toothed cat."s);

	Exit* cliffToPath = new Exit("Cliff to path"s, ""s, cliff, path, NORTH);
	Exit* pathToEnd = new Exit("Path to end"s, ""s, path, end, NORTH);
	Exit* cliffToCave = new Exit("Cave entrance"s, ""s, cliff, cave, EAST);
	Exit* cliffToForest = new Exit("Cliff to forest"s, ""s, cliff, forest, WEST);
	Exit* forestToTree = new Exit("Forest to tree"s, ""s, forest, upTheTree, UP);
	Exit* forestToClearing = new Exit("Forest to clearing"s, ""s, forest, clearing, WEST);
	Exit* forestToWellEntrance = new Exit("Forest to well entrance"s, ""s, forest, wellEntrance, SOUTH);
	Exit* wellEntranceToDownTheWell = new Exit("Well entrance to down the well"s, ""s, wellEntrance, downTheWell, DOWN);
	
	

	
	Creature* monster = new Creature("Smilodon"s, "The Saber-toothed cat"s, end);

	player = new Player("Player"s, "You're a lost hunter and you have to find the way back to your village."s, cliff);

	Item* wallPainting = new Item("Painting"s, "You can see a group of hunters with spears fighting a big mammoth."s, STATIC);

	Item* berry = new Item("Berry"s, "Red, small and juicy fruit."s, FOOD);

	Item* meat = new Item("Meat"s, "Some dead creature's spoiled meat. It looks rotten."s, COMMON);
	Item* branch = new Item("Branch"s, "A long tree limb."s, MATERIAL);
	Item* flint = new Item("Flint"s, "A small piece of sharp flint."s, MATERIAL);
	Item* vine = new Item("Vine"s, "A long string, it could be used as a rope."s, MATERIAL);

	list<string> spearReceipe;
	spearReceipe.push_back(branch->name);
	spearReceipe.push_back(flint->name);
	spearReceipe.push_back(vine->name);

	cliff->AddItem(wallPainting);
	clearing->AddItem(branch);
	cave->AddItem(flint);
	upTheTree->AddItem(vine);
	forest->AddItem(berry);
	downTheWell->AddItem(meat);

	CraftableItem* spear = new CraftableItem("Spear"s, "long and sharp weapon"s, WEAPON, spearReceipe);

	interactables = {player, berry, branch, flint, vine, meat, wallPainting, spear};
	
}

World::~World()
{
}

Action World::ParseInput(const string& input) {

	Command* command;
	string args;
	MovementState movementState;
	bool found = false;

	if (input.size() == 0) {
		return NONE;
	}

	command = GetCommand(input);

	if (command == NULL) {
		return NONE;
	}

	args = command->GetArguments(input);

	switch (command->action) {
		case LOOK:
			if (args.empty()) {
				player->Look(NULL);
			}
			else {
				for (Entity* element : interactables) {
					if (compareString(args, element->name)) {
						player->Look(element);
						found = true;
						break;
					}
				}
				if (!found) {
					printMessage("This item does not exists.");
				}
			}
			break;
		case GO:
			if (args.empty()) {
				movementState = player->Go(NOWHERE);
			}
			else {
				for (Direction direction : directionStrings) {
					if (compareString(args, direction)) {
						movementState = player->Go(direction);
						found = true;
						break;
					}
				}
				if (!found) {
					printMessage("I didn't understand where you want to go.");
					movementState = NOTMOVING;
				}
			}
			if (movementState == ENDING) {
				bool gotSpear = false;
				bool gotMeat = false;
				
				for (Entity* item : player->childEntities) {
					if (compareString(item->name, "Spear")) {
						gotSpear = true;
					}
					else if (compareString(item->name, "Meat")) {
						gotMeat = true;
					}
				}
				if (gotMeat && gotSpear) {
					printMessage("You throw the spoiled meat to distract the creature and attack him from behind. After killing the great Smilodon you go north and find your way back to your village.");
				}
				else if (gotMeat) {
					printMessage("You distract the creature with the meat for a while, but he ends up catching you and you die eaten by the great beast.");
				}
				else if (gotSpear) {
					printMessage("You fight with the great creature. The Smilodon jumps to you while you stab him with your spear. You killed him, but you also end up dying from the wounds");
				}
				else {
					printMessage("With nothing to kill nor distract the beast it crushes you in a heartbeat.");
				}
				command->action = END;
			}
			break;
		case TAKE:
			if (args.empty()) {
				player->Take(NULL);
			}
			else {
				for (Entity* element : interactables) {
					if (compareString(args, element->name)) {
						if (Item* item = dynamic_cast<Item*>(element)) {
							player->Take(item);
							found = true;
							break;
						}
						else {
							printMessage("That's not an item you can take.");
						}
					}
				}
				if (!found) {
					printMessage("This item does not exists.");
				}
			}
			break;
		case DROP:
			if (args.empty()) {
				player->Drop(NULL);
			}
			else {
				for (Entity* element : player->childEntities) {
					if (compareString(args, element->name)) {
						if (Item* item = dynamic_cast<Item*>(element)) {
							player->Drop(item);
							found = true;
							break;
						}
						else {
							printMessage("That's not an item you can drop.");
						}
					}
				}
				if (!found) {
					printMessage("You don't have that item."s);
				}
			}
			break;
		case CRAFT:
			if (args.empty()) {
				player->Craft(NULL);
			}
			else {
				for (Entity* element : interactables) {
					if (compareString(args, element->name)) {
						if (CraftableItem* item = dynamic_cast<CraftableItem*>(element)) {
							player->Craft(item);
							found = true;
							break;
						}
						else {
							printMessage("That's not a craftable item.");
						}
					}
				}
				if (!found) {
					printMessage("This item does not exists.");
				}
			}
			break;
		case EAT:
			if (args.empty()) {
				player->Eat(NULL);
			}
			else {
				for (Entity* element : interactables) {
					if (compareString(args, element->name)) {
						if (Item* item = dynamic_cast<Item*>(element)) {
							player->Eat(item);
							found = true;
							break;
						}
						else {
							printMessage("You can't eat that!");
						}
					}
				}
				if (!found) {
					printMessage("This food does not exists.");
				}
			}
			break;
		case INVENTORY:
			player->Inventory();
			break;
		case QUIT:
		default: 
			return NONE;
	}

	return command->action;
}

Command* World::GetCommand(const string& input) {
	Command* cmd = NULL;
	for (Command* command : commands) {
		if (command->IsCommand(input)) {
			cmd = command;
			break;
		}
	}
	return cmd;
}
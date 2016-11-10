#include <iostream>
#include <string>
#include "world.h"


World::World()
{
	commands = {
		new Command({ "look" }, LOOK, 1),
		new Command({ "go" }, GO, 1),
		new Command({ "take", "pick", "get" }, TAKE, 1),
		new Command({ "drop" }, DROP, 1),
		new Command({ "craft" }, CRAFT, 1),
		new Command({ "eat" }, EAT, 1),
		new Command({ "inventory" }, INVENTORY, 1),
		new Command({ "open" }, OPEN, 1),
		new Command({ "put" }, PUT, 1),
		new Command({ "quit", "exit" }, QUIT, 1),
	};
	
	Room* cliff = new Room("Base of the cliff"s, "You're in the north wall of a big mountain. You can see three paths, one on each direction."s);
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

	Item* wallPainting = new Item("Painting"s, "You can see a group of hunters with spears fighting a big mammoth."s, NULL, STATIC);

	Item* bag = new Item("Bag", "Small leather bag. It may contain something.", NULL, CONTAINER);
	Item* berry = new Item("Berry"s, "Red, small and juicy fruit."s, bag, FOOD);

	Item* meat = new Item("Meat"s, "Some dead creature's spoiled meat. It looks rotten."s, NULL, COMMON);
	Item* branch = new Item("Branch"s, "A long tree limb."s, NULL, MATERIAL);
	Item* flint = new Item("Flint"s, "A small piece of sharp flint."s, NULL, MATERIAL);
	Item* vine = new Item("Vine"s, "A long string, it could be used as a rope."s, NULL, MATERIAL);

	list<string> spearReceipe;
	spearReceipe.push_back(branch->name);
	spearReceipe.push_back(flint->name);
	spearReceipe.push_back(vine->name);

	cliff->AddItem(wallPainting);
	clearing->AddItem(branch);
	cave->AddItem(flint);
	upTheTree->AddItem(vine);
	forest->AddItem(bag);
	downTheWell->AddItem(meat);

	CraftableItem* spear = new CraftableItem("Spear"s, "long and sharp weapon"s, NULL, WEAPON, spearReceipe);

	interactables = {player, bag, berry, branch, flint, vine, meat, wallPainting, spear};

	printMessage("Welcom to Tribal Zork!");
	printMessage("10.000 BC. You wake up at the base of a high cliff. After going hunting with your tribe you felt down, and lost all your equipment. You've probably been unconscious for a few hours. Explore your surroundings and find a way to get back home.");
	player->Look(NULL);

	player->Go(WEST);
	player->Take(bag);
	player->Open(bag);
	player->Take(berry);
	player->Put(berry, bag);
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
					if (compareString(args, element->name) && element->parent == NULL) {
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
					if (compareString(args, element->name) && element->parent == NULL) {
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
					if (compareString(args, element->name) && element->parent == NULL) {
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
		case OPEN:
			if (args.empty()) {
				player->Open(NULL);
			}
			else {
				for (Entity* element : interactables) {
					if (compareString(args, element->name) && element->parent == NULL) {
						if (Item* item = dynamic_cast<Item*>(element)) {
							player->Open(item);
							found = true;
							break;
						}
						else {
							printMessage("You can't open that.");
						}
					}
				}
				if (!found) {
					printMessage("This item does not exists.");
				}
			}
			break;
		case PUT:
			if (args.empty()) {
				player->Put(NULL, NULL);
			}
			else {
				Item* first = NULL;
				Item* second = NULL;
				for (Entity* element : interactables) {
					if (compareString(args.substr(0,element->name.size()), element->name) && element->parent == NULL) {
						if (Item* item = dynamic_cast<Item*>(element)) {
							first = item;
							break;
						}
					}
				}
				if (first == NULL) {
					printMessage("You don't have that first item");
					break;
				}
				//Ignoring first item
				args.erase(0, first->name.size());
				//Ignoring " in "
				args.erase(0, 4);
				for (Entity* element : interactables) {
					if (compareString(args.substr(0, element->name.size()), element->name) && element->parent == NULL) {
						if (Item* item = dynamic_cast<Item*>(element)) {
							second = item;
							break;
						}
					}
				}
				if (first != NULL && second != NULL) {
					player->Put(first, second);
				}
				else {
					printMessage("You don't have that container.");
				}
			}
			break;
		case QUIT:
			break;
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
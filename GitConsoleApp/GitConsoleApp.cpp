// GitConsoleApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <git2.h>
#include <SFML\Graphics.hpp>
#include "TacObject.h"
#include "TacCommit.h"
#include "TacBranch.h"
#include "tac.h"
#include <vector>
using namespace std;
int placeY;
map<git_commit*, vector<git_commit*>> child_Parents;
map<git_commit*, TacCommit*> git_Tac;
// Finds the TacCommit that is derived from the git_Commit "current"
void BuildTacTree()
{
	for (map<git_commit*, vector<git_commit*>>::iterator it = child_Parents.begin(); it != child_Parents.end(); ++it)
	{
		// gets the current child TacObject
		TacCommit* tacChild = git_Tac[it->first];
		int parentsSize = it->second.size();
		for (int i = 0; i < parentsSize; i++)
		{
			git_commit* gitParent = it->second[i];
			TacCommit* tacParent = git_Tac[gitParent];
			tacChild->AddParent(tacParent);
			tacParent->AddChild(tacChild);
		}
	}
}

bool DoesMapContainCommit(git_commit* test)
{
	for (map<git_commit*, vector<git_commit*>>::iterator it = child_Parents.begin(); it != child_Parents.end(); ++it)
	{
		if (it->first == test)
		{
			return true;
		}
	}
	return false;
}

void BuildChild_Parents(git_commit* current) 
{
	if (DoesMapContainCommit(current))
	{
		return;
	}
	const char* commitMessage = git_commit_message(current);
	git_commit* parent = nullptr;
	int parentCount = git_commit_parentcount(current);
	child_Parents.emplace(current, vector<git_commit*>());
	for (int i = 0; i < parentCount; i++)
	{
		git_commit_parent(&parent, current, i);
		child_Parents[current].push_back(parent);
		BuildChild_Parents(parent);
	}
}

void BuildGit_Tac()
{
	for (map<git_commit*, vector<git_commit*>>::iterator it = child_Parents.begin(); it != child_Parents.end(); ++it)
	{
		TacCommit* tacCurrent;
		tacCurrent = new TacCommit(it->first, git_commit_message(it->first), 0, 0);
		git_Tac.emplace(it->first, tacCurrent);
	}
}

int main()
{
	git_libgit2_init();
	vector<TacObject*> objects;
	// instantiate the objects to be displayed
	// open the repository
	git_repository* repo = nullptr;
	int error = git_repository_init(&repo, "C:/Users/Caleb/Documents/Github/TextOverlappingTest", false);
	git_commit* current = nullptr;
	git_commit* parent = nullptr;
	TacCommit* tacCurrent;
	git_oid oid;
	// get all the branches
	git_branch_iterator* it;
	git_branch_iterator_new(&it, repo, GIT_BRANCH_LOCAL);
	git_reference* branch;
	git_branch_t local = GIT_BRANCH_LOCAL;
	while (git_branch_next(&branch, &local, it) != GIT_ITEROVER)
	{
		const char* branchName;
		branchName = git_reference_name(branch);
		git_reference_name_to_id(&oid, repo, branchName);
		git_commit_lookup(&current, repo, &oid);
		placeY = 0;
		BuildChild_Parents(current);
		git_reference_free(branch);
	}
	BuildGit_Tac();
	BuildTacTree();
	git_branch_iterator_free(it);
	for (map<git_commit*, TacCommit*>::iterator it = git_Tac.begin(); it != git_Tac.end(); ++it)
	{
		objects.push_back(it->second);
		it->second->PlaceChildren();
	}
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->FixChildrenWithMultipleParents();
	}
	// start branch iteration again, this time creating the TacBranch objects
	git_branch_iterator_new(&it, repo, GIT_BRANCH_LOCAL);
	while (git_branch_next(&branch, &local, it) != GIT_ITEROVER)
	{
		const char* branchName;
		branchName = git_reference_name(branch);
		git_reference_name_to_id(&oid, repo, branchName);
		git_commit_lookup(&current, repo, &oid);
		TacBranch* tbCurrent = new TacBranch(branchName, git_Tac[current], 0, 0);
		objects.push_back(tbCurrent);
	}
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Tactile Git");
	View v1 = window.getDefaultView();
	window.setView(v1);
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		if (Keyboard::isKeyPressed(Keyboard::Left))
		{
			v1 = window.getView();
			v1.move(-1, 0);
			window.setView(v1);
		}
		if (Keyboard::isKeyPressed(Keyboard::Right))
		{
			v1 = window.getView();
			v1.move(1, 0);
			window.setView(v1);
		}
		if (Keyboard::isKeyPressed(Keyboard::Up))
		{
			v1 = window.getView();
			v1.move(0, -1);
			window.setView(v1);
		}
		if (Keyboard::isKeyPressed(Keyboard::Down))
		{
			v1 = window.getView();
			v1.move(0, 1);
			window.setView(v1);
		}
		window.clear(Color(255, 255, 255));
		for (int i = 0; i < objects.size(); i++)
		{
			objects[i]->DrawLines(window);
		}
		for (int i = 0; i < objects.size(); i++)
		{
			window.draw(*objects[i]);
			if (i != 0)
			{
			}
		}
		window.display();
	}
	for (int i = 0; i < objects.size(); i++)
	{
		delete objects[i];
	}
    return 0;
}
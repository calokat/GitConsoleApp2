// GitConsoleApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <git2.h>
#include <SFML\Graphics.hpp>
#include "TacObject.h"
#include "TacCommit.h"
#include "tac.h"
#include <vector>
using namespace std;
int placeY;
bool AddTacCommit(vector<TacCommit*>& commits, TacCommit * newTacCommit)
{
	for (int i = 0; i < commits.size(); i++)
	{
		if (*commits[i] == *newTacCommit)
		{
			return false;
		}
	}
	commits.push_back(newTacCommit);
	return true;
}

void AttachParent(vector<TacCommit*> &objects, TacCommit* child, int yParam)
{
	git_commit* parent;
	git_commit* gitChild = child->GetGitCommit();
	git_commit_parent(&parent, gitChild, 0);
	for (int i = 0; i < objects.size(); i++)
	{
		// if parent is a parent of gitChild, then
		//we connect the resulting TacCommits
		if (objects[i]->GetGitCommit() == parent)
		{
			for (int j = 0; j < git_commit_parentcount(gitChild); j++)
			{
				git_commit_parent(&parent, gitChild, j);
				TacCommit* tacParent = new TacCommit(parent, git_commit_message(parent), 0, yParam);
				child->AddParent(tacParent);
			}
		}
	}
}

void AttachObjects(vector<TacCommit*>& objects, TacCommit * parent, int yParam)
{
	git_commit* gitChild;
	git_commit* gitParent = parent->GetGitCommit();
	git_commit* testParent;
	for (int i = 0; i < objects.size(); i++)
	{
		gitChild = objects[i]->GetGitCommit();
		if (git_commit_parent(&testParent, gitChild, 0))
		{
			continue;
		}
		if (testParent == gitParent)
		{
			parent->AddChild(objects[i]);
			objects[i]->AddParent(parent);
		}
	}
}

TacCommit* FindExistingTacCommit(vector<TacCommit*> objects, git_commit* current)
{
	for (int i = 0; i < objects.size(); i++)
	{
		if (objects[i]->GetGitCommit() == current)
		{
			return objects[i];
		}
	}
	return nullptr;
}

void BuildTacTree(vector<TacCommit*> &objects, git_commit* current)
{
	TacCommit* tacCurrent = FindExistingTacCommit(objects, current);
	//git_commit* current;
	git_commit* parent;
	int placeX = 0;
	//placeY = 30;
	if (tacCurrent == nullptr)
	{
		tacCurrent = new TacCommit(current, git_commit_message(current), placeX, placeY);
	}
	git_commit_parent(&parent, current, 0);
	printf(git_commit_message(current));
	// exit the function if tacCurrent is already in objects
	//if (!AddTacCommit(objects, tacCurrent))
	//{
	//	return;
	//}
	AddTacCommit(objects, tacCurrent);
	placeY += 300;
	if (objects.size() > 1)
	{
		// do not link tacCurrent its preceding TacCommit if the preceding TacCommit is a root
		// this does not apply when there are only 2 TacCommit* in objects
		// the following two variables are just to see if the previously added commit is a root
		//git_commit* previousCommit = objects[objects.size() - 2]->GetGitCommit();
		//git_commit* parentTest;
		//if (git_commit_parent(&parentTest, previousCommit, 0) == 0)
		//{
		//	objects[objects.size() - 2]->AddParent(tacCurrent);
		//	tacCurrent->AddChild(objects[objects.size() - 2]);
		//}
		//else
		//{
		//AttachParent(objects, tacCurrent, placeY);
		AttachObjects(objects, tacCurrent, placeY);
		//}
	}
	//current = parent;
	//git_oid oid;
	//// get all the branches
	//git_branch_iterator* it;
	//git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL);
	//git_reference* branch;
	//git_branch_t local = GIT_BRANCH_LOCAL;
	//// for each branch, get all parent commits and 
	//// replicate their structure with TacCommits
	//while (git_branch_next(&branch, &local, it) != GIT_ITEROVER)
	//{
		// get the commit branch is pointing at
		//const char* branchName;
		//branchName = git_reference_name(branch);
		//git_reference_name_to_id(&oid, repo, branchName);
		//git_commit_lookup(&current, repo, &oid);
		//placeY = 0;
		// until we reach a root
	for (int i = 0; i < git_commit_parentcount(current); i++)
	{
		//git_commit_parent(&parent, current, i);
		current = parent;
		BuildTacTree(objects, parent);
	}
		//while (git_commit_parentcount(current) != 0)
		//{
		//	if (git_commit_parentcount(current) == 1)
		//	{
		//		tacCurrent = new TacCommit(current, git_commit_message(current), placeX, placeY);
		//		git_commit_parent(&parent, current, 0);
		//		printf(git_commit_message(current));
		//		AddTacCommit(objects, tacCurrent);
		//		//objects.push_back(tacCurrent);
		//		if (objects.size() > 1)
		//		{
		//			tacCurrent->AddChild(objects[objects.size() - 2]);
		//			objects[objects.size() - 2]->AddParent(tacCurrent);
		//		}
		//		placeY += 300;
		//		current = parent;
		//	}
		//}
		//tacCurrent = new TacCommit(current, git_commit_message(current), placeX, placeY);
		//printf(git_commit_message(current));
		//AddTacCommit(objects, tacCurrent);
		////objects.push_back(tacCurrent);
		//tacCurrent->AddChild(objects[objects.size() - 2]);
		//objects[objects.size() - 2]->AddParent(tacCurrent);
	//}
	//git_branch_iterator_free(it);
}

int main()
{
	// instantiate the objects to be displayed
	vector<TacCommit*> objects;
	// open the repository
	git_repository* repo = nullptr;
	int error = git_repository_init(&repo, "C:/Users/Caleb/Desktop/GitTest3", false);
	git_commit* current = nullptr;
	git_commit* parent = nullptr;
	TacCommit* tacCurrent; 
	git_oid oid;
	int placeX = 0;
	placeY = 30;
	// get all the branches
	git_branch_iterator* it;
	git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL);
	git_reference* branch;
	git_branch_t local = GIT_BRANCH_LOCAL;
	while (git_branch_next(&branch, &local, it) != GIT_ITEROVER)
	{
		const char* branchName;
		branchName = git_reference_name(branch);
		git_reference_name_to_id(&oid, repo, branchName);
		git_commit_lookup(&current, repo, &oid);
		placeY = 0;
		BuildTacTree(objects, current);
	}
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->PlaceChildren();
	}
	//BuildTacTree(objects, repo);
	// find all the branches
	/*git_branch_iterator* it;
	git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL);
	git_reference* branch;
	git_branch_t local = GIT_BRANCH_LOCAL;
	while (git_branch_next(&branch, &local, it) != GIT_ITEROVER)
	{
		const char* branchName;
		branchName = git_reference_name(branch);
		git_reference_name_to_id(&oid, repo, branchName);
		git_commit_lookup(&current, repo, &oid);
		placeY = 0;
		while (git_commit_parentcount(current) != 0)
		{
			if (git_commit_parentcount(current) == 1)
			{
				tacCurrent = new TacCommit(current, git_commit_message(current), placeX, placeY);
				git_commit_parent(&parent, current, 0);
				//tacParent = new TacCommit(parent, git_commit_message(parent), placeX, placeY);
				printf(git_commit_message(current));
				//tacParent->AddChild(tacCurrent);
				//tacCurrent->AddParent(tacParent);
				objects.push_back(tacCurrent);
				if (objects.size() > 1)
				{
					tacCurrent->AddChild(objects[objects.size() - 2]);
					objects[objects.size() - 2]->AddParent(tacCurrent);
				}
				/*git_commit_parent(&parent, current, 0);
				current = parent;*/
				/*placeY += 300;
				current = parent;
			}
		}
		tacCurrent = new TacCommit(current, git_commit_message(current), placeX, placeY);
		printf(git_commit_message(current));
		objects.push_back(tacCurrent);
		tacCurrent->AddChild(objects[objects.size() - 2]);
		objects[objects.size() - 2]->AddParent(tacCurrent);
	}
	git_branch_iterator_free(it);*/
	for (int i = 0; i < objects.size(); i++)
	{
		if (i == 0)
		{
			continue;
		}
		//objects[i]->AddParent(objects[i - 1]);
	}
	sf::RenderWindow window(sf::VideoMode(800, 200), "SFML works!");
	View v1 = window.getDefaultView();
	View v2(FloatRect(0, 0, 800, 200));
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
				//DrawLineBetweenObjects(objects[i].GetParent(), &objects[i], window);
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
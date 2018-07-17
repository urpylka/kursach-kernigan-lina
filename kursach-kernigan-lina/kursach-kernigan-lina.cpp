// kursach-kernigan-lina.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <unordered_map>
#include <map>
#include <string>
#include <set>
#include <iostream>
#include <queue> 
#include "omp.h"

#define DEBUG false
using namespace std;
#define COUNT 16
#define CELL_NET_COUNT COUNT/10
#define P 8
int graph[COUNT][COUNT];

set<int> get_mirror_partition(set<int> part) {
	srand(3242);
	set<int> mirror;
	vector<bool> used(COUNT);
	for each (auto var in part)
	{
		used[var] = true;
	}
	for (int i = 0; i < COUNT; i++)
		if (!used[i])
			mirror.insert(i);
	return mirror;
}
int get_solution(set<int> part) {
	int solution = 0;
	for each (auto var in part)
	{
		for (int i = 0; i < COUNT; i++) {
			if (part.find(i) == part.end())
				solution += graph[var][i];
		}
	}
	return solution;
}
void print_solution(set<int> p1, set<int> p2) {
	for each (auto var in p1)
	{
		cout << var << " ";
	}
	cout << endl << "CURRENT SOLUTION: " << get_solution(p1) << endl;
	for each (auto var in p2)
	{
		cout << var << " ";
	}
	cout << endl;
}
void print_partition(set<int> p1) {
	for each (auto var in p1)
	{
		cout << var << " ";
	}
	cout << endl;
}
set<int> get_best_solution(vector<set<int>> solutions) {
	set<int> best = solutions[0];

	for each (auto var in solutions)
	{
		if (get_solution(best) >= get_solution(var))
			best = var;
		if (DEBUG)
#pragma omp critical
		{
			cout << "----bbbest------" << get_solution(var) << endl;
			print_partition(var);
			cout << "----bbbest------" << endl;
		}
	}
	return best;
}
int check_solutions(set<int> new_part, set<int> old_part) {
	return get_solution(old_part) - get_solution(new_part);
}
bool bfs() {
	queue<int> q;
	q.push(0);
	vector<bool> used(COUNT);
	used[0] = true;
	while (!q.empty()) {
		int v = q.front();
		q.pop();
		for (size_t i = 0; i < COUNT; ++i) {
			if (graph[v][i])
				if (!used[i]) {
					used[i] = true;
					q.push(i);
				}
		}
	}
	bool b = true;
	for (size_t i = 0; i < COUNT; ++i) {
		b = b&&used[i];
	}
	return b;
}
int main() {
	double t1, t2;
	set<int> glob_part, new_part, old_part;
	for (int i = 0; i < COUNT; i++) {
		glob_part.insert(i);
	}
	// filling
	for (int i = 0; i < COUNT; i++)
	{
		for (size_t j = 0; j < CELL_NET_COUNT; j++)
		{
			int p = COUNT - i;
			if (!glob_part.empty()) {
				if (glob_part.find(i) != glob_part.end()) {
					glob_part.erase(i);

					if (glob_part.size() >= COUNT - CELL_NET_COUNT)
					{
						while (glob_part.find(p) == glob_part.end() && p != i)
							p = rand() % COUNT;
					}
					else
					{
						while (glob_part.find(p) != glob_part.end() && p != i)
							p = rand() % COUNT;
					}
					glob_part.erase(p);
					graph[i][p] = graph[p][i] = 1;
				}
			}
			while (graph[i][p] || p == i)
			{
				p = rand() % COUNT;
			}
			graph[i][p] = graph[p][i] = 1;
		}
	}
	// checking
	if (bfs())
		cout << "SVYZNII" << endl;
	else
		cout << "NE SVYZNII" << endl;

	if (DEBUG)
	{
		for (int i = 0; i < COUNT; i++)
		{
			for (int j = 0; j < COUNT - 1; j++) {
				cout << graph[i][j] << " ";
			}
			cout << graph[i][COUNT - 1] << endl;
		}
		cout << endl;
	}
	for (int i = 0; i < COUNT; i++) {
		glob_part.insert(i);
	}

	// random partioning
	int p1 = 0; int p2 = COUNT - 1;
	while (true)
	{
		while (glob_part.find(p1) == glob_part.end())
			p1 = rand() % COUNT;
		glob_part.erase(p1);
		new_part.insert(p1);
		while (glob_part.find(p2) == glob_part.end())
			p2 = rand() % COUNT;
		glob_part.erase(p2);
		old_part.insert(p2);
		if (glob_part.empty())break;
	}
	if (DEBUG)
	{
		print_solution(new_part, old_part);
	}
	int iteration = 0;
	int random_solution = get_solution(new_part);
	t1 = omp_get_wtime();
	while (true) {

		set<int> tmp1, tmp2;
		vector<set<int>> best_solutions(P), temps(P);
		for each (auto var in new_part)
		{
			for (int i = 0; i < P; i++)
			{
				temps[i].insert(var);
				best_solutions[i].insert(var);
			}
			tmp1.insert(var);
		}
		vector<queue<pair<int, int>>> swap_queues(P);
		vector<set<int>> set_for_threads(P);
		int i = 0;
		for each (auto var in get_mirror_partition(tmp1))
		{
			while (tmp1.find(p2) == tmp1.end())
				p2 = rand() % COUNT;
			tmp1.erase(p2);
			swap_queues[i%P].push(pair<int, int>(p2, var));
			i++;
		}

		for each (auto var in new_part)
		{
			tmp1.insert(var);
		}
#pragma omp parallel for firstprivate(tmp1,i) 
		for (i = 0; i < P; i++) {
			if (DEBUG)
#pragma omp critical
			{
				cout << "----bbbest------" << i << endl;
				print_partition(best_solutions[i]);
				cout << "----bbbest------" << endl;
			}
			int idx = 0;
			while (!swap_queues[i].empty()) {
				pair<int, int> swap_pair = swap_queues[i].front();
				swap_queues[i].pop();
				temps[i].erase(swap_pair.first);
				temps[i].insert(swap_pair.second);
				if (!idx)best_solutions[i] = temps[i];
				if (DEBUG)
#pragma omp critical
				{
					cout << "-------------------------------------------" << endl;
					cout << "SOLUTION FOR " << i << endl;
					print_partition(temps[i]);

					cout << "" << get_solution(temps[i]) << endl;
					cout << "-------------------------------------------" << endl;
					cout << "BEST " << get_solution(best_solutions[i]) << endl;
					print_partition(best_solutions[i]);
					cout << endl;
					cout << "-------------------------------------------" << endl;
					cout << "SWAP PAIR " << swap_pair.first << " " << swap_pair.second << endl;
					cout << "-------------------------------------------" << endl;
					cout << "LEN SET" << temps[i].size() << endl;
					cout << "-------------------------------------------" << endl << endl << endl;

				}
				if (get_solution(temps[i]) <= get_solution(best_solutions[i]))
				{
					best_solutions[i].erase(swap_pair.first);
					best_solutions[i].insert(swap_pair.second);
				}

				temps[i].insert(swap_pair.first);
				temps[i].erase(swap_pair.second);
				if (DEBUG)
#pragma omp critical
				{
					cout << "-------reset-----" << i << endl;
					print_partition(temps[i]);

					cout << "" << get_solution(temps[i]) << endl;
					cout << "-------reset-----" << endl;

					system("pause");
				}
			}
		}
		set<int> best_solution = get_best_solution(best_solutions);
		int diff = check_solutions(best_solution, new_part);
		if (DEBUG) {
			if (true && diff != 0) {
				cout << endl << "ITERATION: " << iteration << endl << "CURRENT SOLUTION DIFF: " << diff << endl;
			}
			iteration++;
			if (diff < 0)break;
			new_part.clear();
			for each (auto var in best_solution)
			{
				new_part.insert(var);
			}
			if (DEBUG)
				print_solution(best_solution, get_mirror_partition(best_solution));
		}
		t2 = omp_get_wtime();
		if (DEBUG)
			print_solution(new_part, get_mirror_partition(new_part));

		cout << "Time: " << t2 - t1 << endl;
		cout << "SOLURION: " << get_solution(new_part) << endl;
		cout << "DIFF: " << random_solution - get_solution(new_part) << endl;
		cout << "ITERATIONS: " << iteration << endl;
		system("pause");
		return 0;
	}
}


#include <iostream>
#include <vector>
#include <random>
#include <fstream>

using namespace std;

random_device rd; //Needed better random number generation for really small probabilities
mt19937 mt(rd()); // unfortunately that means they are not deterministic
uniform_real_distribution<double> dist(0.0, nextafter(1, DBL_MAX));
const int L = 1000; // Lattice size is L*L
const int N = 10000000; // Number of agents
vector<vector<vector<int>>> lattice; // vector of people in x,y site ; 0: susceptible, 1: infected, 2: recovered
double beta = 0.01; // Chance of infection (at start)
double nu = 0.04; // Chance of recovery
double infected = 0.00000025; // infected ratio at the start
int Infected, Recovered;
double random_walk_probability = 0.4; //initially
int walks = 4; //initially
int days = 74; // number of cycles is days * walks


vector<int>& Lattice(int x, int y) { // gets reference of an element from the lattice, enforces periodicity
	if (x > (L - 1))x = x % L;
	if (y > (L - 1))y = y % L;
	if (x < 0)x = (L - 1) + x;
	if (y < 0)y = (L - 1) + y;
	return lattice[x][y];
}

void make_lattice(vector<vector<vector<int>>>& m_lattice, int size) { // empty lattice
	vector<int> site;
	vector<vector<int>> column;
	for (int i = 0; i < size; i++) {
		column.clear();
		for (int j = 0; j < size; j++) {
			column.push_back(site);
		}
		m_lattice.push_back(column);
	}
}

void make_agents(vector<vector<vector<int>>>& ma_lattice, int n, double infection_chance) { // adding n agents, n*i_c is infected
	double agent_chance = (double)n / (L*L);
	if (agent_chance > 1)agent_chance = 0.8;
	while (n != 0) {
		for (int i = 0; i < ma_lattice.size(); i++) {
			for (int j = 0; j < ma_lattice[i].size(); j++) {
				if (agent_chance > dist(mt)) {
					if (infection_chance > dist(mt)){
						ma_lattice[i][j].push_back(1);
						Infected++;
						if (--n == 0)return;
					}
					else
					{
						ma_lattice[i][j].push_back(0);
						if (--n == 0)return;
					}
				}
			}
		}
	}
	return;
}

void random_walk(vector<vector<vector<int>>>& rw_lattice, double walk_prob) {
	for (int i = 0; i < rw_lattice.size(); i++) {
		for (int j = 0; j < rw_lattice[i].size(); j++) {
			for (int k = 0; k < rw_lattice[i][j].size(); k++) {
				if (walk_prob > dist(mt)) {
					switch (rand() % 4)
					{
					case 0:
						Lattice(i + 1,j).push_back(rw_lattice[i][j][k]); //move
						rw_lattice[i][j].erase(rw_lattice[i][j].begin() + k);
							break;
					case 1:
						Lattice(i - 1, j).push_back(rw_lattice[i][j][k]);
						rw_lattice[i][j].erase(rw_lattice[i][j].begin() + k);
						break;
					case 2:
						Lattice(i, j + 1).push_back(rw_lattice[i][j][k]);
						rw_lattice[i][j].erase(rw_lattice[i][j].begin() + k);
						break;
					case 3:
						Lattice(i, j - 1).push_back(rw_lattice[i][j][k]);
						rw_lattice[i][j].erase(rw_lattice[i][j].begin() + k);
						break;
					}
				}
			}
		}
	}
}

void infection_recovery(vector<vector<vector<int>>>& ir_lattice, double infection_rate, double recovery_rate) {
	vector<int> susceptibles;
	int Is_infected = 0; // 1 if there is at least 1 infected on a site
	for (int i = 0; i < ir_lattice.size(); i++) {
		for (int j = 0; j < ir_lattice[i].size(); j++) {
			Is_infected = 0;
			susceptibles.clear();
			for (int k = 0; k < ir_lattice[i][j].size(); k++) {
				if(ir_lattice[i][j][k] == 1){
					Is_infected = 1;
					if (recovery_rate > dist(mt)) {
						ir_lattice[i][j][k] = 2;
						Recovered++;
						Infected--;
					}
				}
				else if (ir_lattice[i][j][k] == 0)susceptibles.push_back(k);
			}
			if(Is_infected){
				for (int kk = 0; kk < susceptibles.size(); kk++) {
					if (infection_rate > dist(mt)) {
						ir_lattice[i][j][kk] = 1;
						Infected++;
					}
				}
			}
		}
	}
}



int main(){
	ofstream file;
	file.open("output.csv");
	make_lattice(lattice, L);
	make_agents(lattice, N ,infected);
	cout << "Total population: " << N << endl;
	cout << "Infected: " << "\t" << "Recovered:" << "\t" << "Total cases:" << endl;
	file << "Infected" << ";" << "Recovered" << ";" << "Total cases" << endl;
	cout << Infected << "\t\t" << Recovered << "\t\t" << Infected + Recovered << endl;
	file << Infected << ";" << Recovered << ";" << Infected + Recovered << endl;
	for (int i = 0; i < days; i++) {
		/*if (i == 14) { //#Stay@home around march 18
			random_walk_probability = random_walk_probability / 4;
			walks = 2; 
		}
		if (i == 56) { //partial easement april 29
			walks = 3;
		}
		beta = beta - 0.0002; //average temperature getting warmer*/
		for (int j = 0; j < walks; j++) {
			random_walk(lattice, random_walk_probability);
			infection_recovery(lattice,beta,nu);
		}
		cout << Infected << "\t\t" << Recovered << "\t\t" << Infected + Recovered << endl;
		file << Infected << ";" << Recovered << ";" << Infected + Recovered << endl;
	}
	file.close();
}


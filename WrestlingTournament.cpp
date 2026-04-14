/// Forrest Knight

/// CS 361

/// Wrestling Tournament Simulation


#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <set>
#include <vector>

using namespace std;

static mt19937 rng(random_device{}());

int weightClasses[13] = {
      94,
     106,
     113,
     120,
     126,
     132,
     138,
     145,
     152,
     167,
     189,
     220,
     285
};

struct Record
{
	int wins = 0;
	int losses = 0;
};

class Wrestler
{
public:
	Wrestler() : id(0), abilityScore(0), wtClass(0), weight(0) {}
	Wrestler(int weightClass, int wt, int ID, int abilitySc) :
		id(ID),
		abilityScore(abilitySc),
		wtClass(weightClass),
		weight(wt)
	{
		record.wins = 0;
		record.losses = 0;
	}
	int id;
	int abilityScore;
	Record record;
	int wtClass;
	int weight;
};

class School
{
public:
	School(int Name, int numWrs):name(Name), numberOfWrestlers(numWrs)
	{
		wrestlers.resize(numberOfWrestlers);
		set<int> wtClasses;

		normal_distribution<double> distributionAbility(100, 15);
		uniform_int_distribution<int> wtDist(94, 285);
		for (int i = 0; i < numberOfWrestlers; ++i)
		{
			int cl = 0;
			int wt = 0;
			while (wtClasses.find(cl) != wtClasses.end() || cl < 1)
			{
				wt = wtDist(rng);
				for (int j = 1; j < 13; ++j)
				{
					if (wt > weightClasses[j - 1] && wt <= weightClasses[j])
					{
						cl = j;
						break;
					}
				}
			}
			int abilityScore = max(static_cast<int>(round<int>(distributionAbility(rng))), 0);
			wrestlers[i] = Wrestler(cl, wt, name + i + 1, abilityScore);
			wtClasses.insert(cl);
		}
	};

	int getWrestler(int wtClass)
	{
		for (int i = 0; i < numberOfWrestlers; ++i)
			if (wrestlers[i].wtClass == wtClass)
				return i;
		return -1;
	}

	float totalSchoolScore()
	{
		int wins = 0;
		int loses = 0;
		for (int i = 0; i < numberOfWrestlers; ++i)
		{
			wins += wrestlers[i].record.wins;
			loses += wrestlers[i].record.losses;
		}

		return float(wins) / (wins + loses);
	}

	int name;
	int numberOfWrestlers;
	vector<Wrestler> wrestlers;
};

class Conference
{
public:
	Conference() : bestScore(0.0f)
	{
		//std::normal_distribution<double> distribution(10, 1);
		for (int schoolName = 100; schoolName < 900; schoolName += 100)
		{
			int numberOfWrestlers = 12;//min(static_cast<int>(round<int>(distribution(rng))), 12);
			schools.push_back(School(schoolName, numberOfWrestlers));
		}

		for (int i = 0; i < 12; ++i)
			for (int j = 0; j < 8; ++j)
				for (int k = 0; k < 8; ++k)
					advantageMatrix[i][j][k] = 0;
	}
	void simulateSeason()
	{
		for (int i = 0; i < 8; ++i)
			for (int j = i + 1; j < 8; ++j)
				schoolsMatch(i, j);
	}

	int bestSchool()
	{
		int bestSchool = -1;
		for (int i = 0; i < 8; ++i)
		{
			if (schools[i].totalSchoolScore() > bestScore)
			{
				bestScore = schools[i].totalSchoolScore();
				bestSchool = (i + 1) * 100;
			}
		}

		return bestSchool;
	}

	int wrestlersMatch(int wrestler1, int wrestler2)
	{
		if (wrestler1 % 100 < 1 || wrestler2 % 100 < 1)
			return 0;
		Wrestler& w1 = schools[wrestler1 / 100 - 1].wrestlers[wrestler1 % 100 - 1];
		Wrestler& w2 = schools[wrestler2 / 100 - 1].wrestlers[wrestler2 % 100 - 1];

		float sigma = max(abs(w1.abilityScore - w2.abilityScore) / 3.f, 15.f);

		std::normal_distribution<double> distribution1(w1.abilityScore, sigma);
		std::normal_distribution<double> distribution2(w2.abilityScore, sigma);
		float score1 = distribution1(rng);
		float score2 = distribution2(rng);

		if (score1 > score2)
		{
			w1.record.wins++;
			w2.record.losses++;
			advantageMatrix[w1.wtClass][wrestler1 / 100 - 1][wrestler2 / 100 - 1]++;
			advantageMatrix[w1.wtClass][wrestler2 / 100 - 1][wrestler1 / 100 - 1]--;
			return 1;
		}
		else
		{
			w1.record.losses++;
			w2.record.wins++;
			advantageMatrix[w1.wtClass][wrestler1 / 100 - 1][wrestler2 / 100 - 1]--;
			advantageMatrix[w1.wtClass][wrestler2 / 100 - 1][wrestler1 / 100 - 1]++;
			return -1;
		}
	}
	void schoolsMatch(int school1, int school2)
	{
		for (int i = 1; i < 13; ++i)
		{
			int w1 = 100 * (school1 + 1) + schools[school1].getWrestler(i) + 1;
			int w2 = 100 * (school2 + 1) + schools[school2].getWrestler(i) + 1;
			wrestlersMatch(w1, w2);
		}
	}
	vector<School> schools;
	float bestScore;
	int advantageMatrix[13][8][8];
};

struct Comp {
	Comp(Conference& conf) { conference = &conf; }
	bool operator()(Wrestler w1, Wrestler w2)
	{
		if (w1.record.wins < w2.record.wins)
			return true;
		if (w2.record.wins < w1.record.wins)
			return false;

		if (conference->advantageMatrix[w1.wtClass][w1.id / 100 - 1][w2.id / 100 - 1] < 0)
			return true;
		else
			return false;

	}
	Conference* conference;
};

class Match
{
public:
	Match(Wrestler& wr1, Wrestler& wr2) : w1(wr1), w2(wr2)
	{
		float sigma = max(abs(w1.abilityScore - w2.abilityScore) / 3.f, 15.f);

		std::normal_distribution<double> distribution1(w1.abilityScore, sigma);
		std::normal_distribution<double> distribution2(w2.abilityScore, sigma);
		float score1 = distribution1(rng);
		float score2 = distribution2(rng);
		if (score1 > score2)
		{
			winner = w1;
			loser = w2;
		}
		else
		{
			winner = w2;
			loser = w1;
		}
	}
	Wrestler w1;
	Wrestler w2;
	Wrestler winner;
	Wrestler loser;
};

class Tournament
{
public:
	Tournament(Conference& conference, int weightClass): conf(&conference), wtClass(weightClass)
	{
		for (int i = 0; i < 8; ++i)
		{
			int id = (*conf).schools[i].getWrestler(weightClass);
			if (id > -1)
			    participants.push_back((*conf).schools[i].wrestlers[id % 100]);
		}
		seed();
	}

	void seed()
	{
		Comp comp(*conf);
		sort(participants.begin(), participants.end(), comp);
	}

	void simulate()
	{
		if (participants.size() < 8)
		{
			cout << "Not enough participants in weight class " << wtClass << " to run tournament." << endl;
			return;
		}
		matches.push_back(Match(participants[0], participants[7]));
		matches.push_back(Match(participants[4], participants[3]));
		matches.push_back(Match(participants[2], participants[5]));
		matches.push_back(Match(participants[1], participants[6]));
		matches.push_back(Match(matches[0].winner, matches[1].winner));
		matches.push_back(Match(matches[2].winner, matches[3].winner));
		matches.push_back(Match(matches[0].loser, matches[1].loser));
		matches.push_back(Match(matches[2].loser, matches[3].loser));
		matches.push_back(Match(matches[4].loser, matches[7].winner));
		matches.push_back(Match(matches[5].loser, matches[6].winner));
		matches.push_back(Match(matches[6].loser, matches[7].loser));
		matches.push_back(Match(matches[8].loser, matches[9].loser));
		matches.push_back(Match(matches[9].winner, matches[10].winner));
		matches.push_back(Match(matches[5].winner, matches[6].winner));
	}
	void printWinner()
	{
		if (matches.size() < 14)
			return;
		cout << "And the winner in the weight class " << wtClass << " is number " << matches[13].winner.id << endl;
	}
	vector<Wrestler> participants;
	vector<Match> matches;
	Conference* conf;
	int wtClass;
};

int main()
{
	Conference conf;
	for (int i = 0; i < 2; ++i)
        conf.simulateSeason();

	cout << "Conference winner: the team of " << conf.bestSchool() << " school with the score of ";
	cout.precision(3);
	cout << conf.bestScore << "! Congrats!" << endl;

	for (int i = 1; i < 13; ++i)
	{
		Tournament tournament(conf, i);
		tournament.simulate();
		tournament.printWinner();
	}
    return 0;
}


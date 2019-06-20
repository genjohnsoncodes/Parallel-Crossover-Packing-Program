#include "../MaxRectsBinPack.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <cstdlib>
using namespace std;
using namespace rbp;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static int POPULATION=100;
static int MUTATION=3; //Number of mutations equals this number-1
static bool ROTATE = 1;

struct rectangles{
	int width;
	int height;
	int area;
	int number;
};

bool inThisArray(int array[], int size, int check);


int main(int argc, char **argv){


	// Create a bin to pack to, use the bin size from command line.
	MaxRectsBinPack bin;
	int c = 0;
	ifstream dataset;
	ofstream out_file;
	char *in_file_name = NULL;
	char *output_file_name = NULL;


	// -i: is the input file
	// -o: is the logfile the will contain stats
	// -m: the number of mutations that will be preformed on the population
	// -p: the population size that for each generation
	// -r: if the rectanlges will be rotated, 1 = yes, 0 = no
	while ((c = getopt(argc, argv, "i:o:p:m:r:")) != -1)
	{
		switch (c)
		{
		case 'i':
			if (optarg)
			{
				in_file_name = optarg;
				dataset.open(in_file_name, ios::in | ios::binary);
				if (!dataset)
				{
					cerr << "Error opening file\a\a\n";
					exit(101);
				}
			}
			break;

		case 'o':
			if (optarg)
			{
				output_file_name = optarg;
				out_file.open(output_file_name, ios::out | ios::binary);
				if (!out_file)
				{
					cout << "\nCannot open " << output_file_name << endl;
					exit(101);
				}
			}
			break;
		case 'm':
			if (optarg)
				MUTATION = atoi(optarg);
			break;
		case 'p':
			if (optarg)
				POPULATION = atoi(optarg);
			break;
		case 'r':
			if (optarg)
				ROTATE = atoi(optarg);
			break;
		}
	}

	if(in_file_name == NULL)
	{
		dataset.open("datasets/c5p2.txt");
	}
	int binWidth;
	int binHeight;
	int numOfRects;
	int misfit = 0;
	float fitness[POPULATION];
	float mostFit = 0;
	int whichFit = 1;

	//Inializes bin width and heigh and number of rectangles from the datasets
	dataset >> binWidth >> binHeight >> numOfRects;

	//Declares rectangle array based on number of rectangles in each dataset
	rectangles rects1[POPULATION][numOfRects];
	rectangles rects2[POPULATION][numOfRects];
	rectangles initRects[numOfRects];

	for (int i = 0; i < numOfRects; i++)
	{
		dataset >> rects1[0][i].width >> rects1[0][i].height;
		initRects[i].width = rects1[0][i].width;
		initRects[i].height = rects1[0][i].height;
		rects1[0][i].area = rects1[0][i].width * rects1[0][i].height;
		initRects[i].area = rects1[0][i].area;
		rects1[0][i].number = i;
		initRects[i].number = rects1[0][i].number;
	}

	////////////////////////////////////////////////////////////////////////
	//Sorting by area with bubble sort
	////////////////////////////////////////////////////////////////////////
	int i, j;
	bool swapped;
	for (i = 0; i < numOfRects - 1; i++)
	{
		swapped = false;
		for (j = 0; j < numOfRects - i - 1; j++)
		{
			if (rects1[0][j].area < rects1[0][j + 1].area)
			{
				rectangles temp = rects1[0][j];
				rects1[0][j] = rects1[0][j + 1];
				rects1[0][j + 1] = temp;
				swapped = true;
			}
		}
		// IF no two elements were swapped by inner loop, then break
		if (swapped == false)
			break;
	}

	printf("Initializing bin to size %dx%d.\n", binWidth, binHeight);
	if (ROTATE == 1)
		bin.Init(binWidth, binHeight);

	else
		bin.Init(binWidth, binHeight, 0);

	////////////////////////////////////////////////////////////////////////
	//AREA HEURISTIC RESULTS
	////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < numOfRects; i++)
	{
		// Read next rectangle to pack.
		int rectWidth = rects1[0][i].width;
		int rectHeight = rects1[0][i].height;

		printf("Rectangle %d of size %dx%d: ", rects1[0][i].number, rectWidth, rectHeight);

		// Perform the packing.
		MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBottomLeftRule; // This can be changed individually even for each rectangle packed.
		Rect packedRect = bin.Insert(rectWidth, rectHeight, heuristic);

		// Test success or failure.
		if (packedRect.height > 0)
		{

			if (rectWidth == packedRect.width)
				printf("No rotation. Packed to (x,y)=(%d,%d), Wasted space: %.2f%%\n", packedRect.x, packedRect.y, 100.f - bin.Occupancy() * 100.f);

			else
				printf("\x1b[31mRotation=%dx%d.\x1b[0m Packed to (x,y)=(%d,%d), Wasted space: %.2f%%\n", packedRect.width, packedRect.height, packedRect.x, packedRect.y, 100.f - bin.Occupancy() * 100.f);
		}
		else
		{
			printf("Failed! Could not find a proper position to pack this rectangle into. Skipping this one.\n");
			misfit++;
		}
	}
	printf("Done. All rectangles processed for area heuristic!\n");
	printf("Number of misfits: %d\n\n", misfit);
	misfit = 0;

	////////////////////////////////////////////////////////////////////////
	//INITIALIZE FIRST POPULATION
	////////////////////////////////////////////////////////////////////////
	srand(time(0));
	for (int i = 0; i < POPULATION; i++)
	{
		for (int j = 0; j < numOfRects; j++)
		{
			rects1[i][j] = initRects[(rand() % numOfRects) + 0];

			if (j > 0)
			{
				for (int k = 0; k < j; k++)
				{
					while (rects1[i][j].number == rects1[i][k].number)
					{
						rects1[i][j] = initRects[(rand() % numOfRects) + 0];
						k = 0;
					}
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////
	//EVO TIME
	////////////////////////////////////////////////////////////////////////
	float open_free_space = 0.00;
	if(output_file_name != NULL)
	{
		out_file << "Current Test: " << in_file_name << endl;
		out_file << "Current Gen" << "\t" << "| Free space (%) | Misfits | Chromosome (X, Y)" << endl;
	}
	for (int evo = 0; evo < 10001; evo++)
	{
		int misfits_array[POPULATION];
		for (int i = 0; i < POPULATION; i++)
		{
			MaxRectsBinPack bin;
			if (ROTATE == 1)
				bin.Init(binWidth, binHeight);

			else
				bin.Init(binWidth, binHeight, 0);
			for (int j = 0; j < numOfRects; j++)
			{

				// Read next rectangle to pack.
				int rectWidth = rects1[i][j].width;
				int rectHeight = rects1[i][j].height;

				if (i == 0)
				{
					printf("Rectangle %d of size %dx%d: ", rects1[i][j].number, rectWidth, rectHeight);
				}

				// Perform the packing.
				MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBottomLeftRule; // This can be changed individually even for each rectangle packed.
				Rect packedRect = bin.Insert(rectWidth, rectHeight, heuristic);

				// Test success or failure.
				if (packedRect.height > 0)
				{
					if (i == 0)
					{

						if (rectWidth == packedRect.width)
							printf("No rotation. Packed to (x,y)=(%d,%d), Wasted space: %.2f%%\n", packedRect.x, packedRect.y, 100.f - bin.Occupancy() * 100.f);

						else
							printf("\x1b[31mRotation=%dx%d.\x1b[0m Packed to (x,y)=(%d,%d), Wasted space: %.2f%%\n", packedRect.width, packedRect.height, packedRect.x, packedRect.y, 100.f - bin.Occupancy() * 100.f);

					}

					continue;
				}

				else
				{
					if (i == 0)
						printf("Failed! Could not find a proper position to pack this rectangle into. Skipping this one.\n");
					misfit++;
				}
			}
			if (i == 0)
			{
				printf("Number of misfits: %d\n\n", misfit);
				cout << evo * 2 << endl;
			}

			fitness[i] = bin.Occupancy() * 100.f;
			misfits_array[i] = misfit;

			if (fitness[i] >= mostFit)
			{
				mostFit = fitness[i];
				whichFit = i;
				if (bin.Occupancy() == 1.0 || evo == 10000)
				{
					if (i != 0)
						continue;

					if (misfit == 0)
						cout << "All rectangles packed!" << endl;

					if(output_file_name != NULL)
					{
						out_file << evo * 2 << "\t\t\t  " << (100 - mostFit) << "\t\t\t\t\t" << misfits_array[whichFit] << "\t\t\t";
						for (int index = 0; index < numOfRects; index++)
						{
							out_file << "(" << rects1[whichFit][index].number << "), ";
						}
						out_file << endl;
					}
					return 0;
				}
			}
			misfit = 0;



		}

		if (open_free_space < mostFit && output_file_name != NULL)
		{
			open_free_space = mostFit;
			out_file << evo * 2 << "\t\t\t  " << (100 - open_free_space) << "\t\t\t\t\t" << misfits_array[whichFit] << "\t\t\t";
			for (int index = 0; index < numOfRects; index++)
			{
				out_file << "(" << rects1[whichFit][index].number << "), ";
			}
			out_file << endl;
		}

		mostFit = 0;

		//Carries over most fit chromosome
		for (int i = 0; i < numOfRects; i++)
		{
			rects2[0][i] = rects1[whichFit][i];
		}

		////////////////////////////////////////////////////////////////
		//Mutation
		////////////////////////////////////////////////////////////////
		int m1;
		int m2;
		for (int a = 1; a < MUTATION; a++)
		{
			m1 = rand() % numOfRects;
			m2 = rand() % numOfRects;
			while (m1 == m2)
			{
				m2 = rand() % numOfRects;
			}
			for (int i = 0; i < numOfRects; i++)
			{
				if (i == m1)
				{
					rects2[a][i] = rects2[0][m2];
				}
				else if (i == m2)
				{
					rects2[a][i] = rects2[0][m1];
				}
				else
					rects2[a][i] = rects2[0][i];
			}
		}

		////////////////////////////////////////////////////////////////////////
		//CROSSOVER W/ 4-TEAM TOURNAMENT
		////////////////////////////////////////////////////////////////////////
		//4-Team Tourny
		for (int cross = MUTATION; cross < POPULATION; cross++)
		{
			int p1 = (rand() % POPULATION) + 0;
			int p2 = (rand() % POPULATION) + 0;
			int p3 = (rand() % POPULATION) + 0;
			int p4 = (rand() % POPULATION) + 0;
			int w1;
			int w2;
			while (p1 == p2)
			{
				p2 = (rand() % POPULATION) + 0;
			}
			while (p1 == p3 || p2 == p3)
			{
				p3 = (rand() % POPULATION) + 0;
			}
			while (p1 == p4 || p2 == p4 || p3 == p4)
			{
				p4 = (rand() % POPULATION) + 0;
			}
			if (fitness[p1] < fitness[p2])
				w1 = p1;
			else
				w1 = p2;
			if (fitness[p3] < fitness[p4])
				w2 = p3;
			else
				w2 = p4;

			int samesies[numOfRects];
			int position[numOfRects];
			int sameCount = 0;
			int sameFinal;

			//Finds matching rects in winning chromosomes
			for (int i = 0; i < numOfRects; i++)
			{
				if (rects1[w1][i].number == rects1[w2][i].number)
				{
					samesies[sameCount] = rects1[w1][i].number;
					position[sameCount] = i;
					rects2[cross][i] = rects1[w1][i];
					sameCount++;
				}
			}

			sameFinal = sameCount;
			sameCount = 0;

			//Crossover
			for (int i = 0; i < numOfRects; i++)
			{
				if (inThisArray(position, sameFinal, i))
				{
					continue;
				}

				else
				{
					rects2[cross][i] = initRects[(rand() % numOfRects) + 0];

					if (i == 0)
					{
						while (inThisArray(samesies, sameFinal, rects2[cross][i].number))
							rects2[cross][i] = initRects[(rand() % numOfRects) + 0];
					}

					if (i > 0)
					{
						for (int k = 0; k < i; k++)
						{
							while (rects2[cross][i].number == rects2[cross][k].number || inThisArray(samesies, sameFinal, rects2[cross][i].number))
							{
								rects2[cross][i] = initRects[(rand() % numOfRects) + 0];
								k = 0;
							}
						}
					}
				}
			}
		}

		//Evaluate second generation
		for (int i = 0; i < POPULATION; i++)
		{
			MaxRectsBinPack bin;

			if (ROTATE == 1)
				bin.Init(binWidth, binHeight);

			else
				bin.Init(binWidth, binHeight, 0);

			for (int j = 0; j < numOfRects; j++)
			{

				// Read next rectangle to pack.
				int rectWidth = rects2[i][j].width;
				int rectHeight = rects2[i][j].height;

				// Perform the packing.
				MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = MaxRectsBinPack::RectBottomLeftRule;
				Rect packedRect = bin.Insert(rectWidth, rectHeight, heuristic);

				// Test success or failure.
				if (packedRect.height > 0)
					continue;
				else
				{
					misfit++;
				}
			}

			fitness[i] = bin.Occupancy();
			if (fitness[i] >= mostFit)
			{
				mostFit = fitness[i];
				whichFit = i;
			}
			misfit = 0;
		}

		mostFit = 0;
		////////////////////////////////////////////////////////////////////////
		//Second back to first
		////////////////////////////////////////////////////////////////////////

		//Carries over most fit chromosome
		for (int i = 0; i < numOfRects; i++)
		{
			rects1[0][i] = rects2[whichFit][i];
		}

		////////////////////////////////////////////////////////////////
		//Mutation
		////////////////////////////////////////////////////////////////
		for (int a = 1; a < MUTATION; a++)
		{
			m1 = rand() % numOfRects;
			m2 = rand() % numOfRects;
			while (m1 == m2)
			{
				m2 = rand() % numOfRects;
			}
			for (int i = 0; i < numOfRects; i++)
			{
				if (i == m1)
				{
					rects1[a][i] = rects1[0][m2];
				}
				else if (i == m2)
				{
					rects1[a][i] = rects1[0][m1];
				}
				else
					rects1[a][i] = rects1[0][i];
			}
		}

		////////////////////////////////////////////////////////////////////////
		//CROSSOVER W/ 4-TEAM TOURNAMENT
		////////////////////////////////////////////////////////////////////////

		//4-Team Tourny
		for (int cross = MUTATION; cross < POPULATION; cross++)
		{
			int p1 = (rand() % POPULATION) + 0;
			int p2 = (rand() % POPULATION) + 0;
			int p3 = (rand() % POPULATION) + 0;
			int p4 = (rand() % POPULATION) + 0;
			int w1;
			int w2;
			while (p1 == p2)
			{
				p2 = (rand() % POPULATION) + 0;
			}
			while (p1 == p3 || p2 == p3)
			{
				p3 = (rand() % POPULATION) + 0;
			}
			while (p1 == p4 || p2 == p4 || p3 == p4)
			{
				p4 = (rand() % POPULATION) + 0;
			}
			if (fitness[p1] < fitness[p2])
				w1 = p1;
			else
				w1 = p2;
			if (fitness[p3] < fitness[p4])
				w2 = p3;
			else
				w2 = p4;

			int samesies[numOfRects];
			int position[numOfRects];
			int sameCount = 0;
			int sameFinal;

			//Finds matching rects in winning chromosomes
			for (int i = 0; i < numOfRects; i++)
			{
				if (rects2[w1][i].number == rects2[w2][i].number)
				{
					samesies[sameCount] = rects2[w1][i].number;
					position[sameCount] = i;
					rects1[cross][i] = rects2[w1][i];
					sameCount++;
				}
			}

			sameFinal = sameCount;
			sameCount = 0;

			//Crossover
			for (int i = 0; i < numOfRects; i++)
			{

				if (inThisArray(position, sameFinal, i))
				{

					continue;
				}

				else
				{
					rects1[cross][i] = initRects[(rand() % numOfRects) + 0];
					if (i == 0)
					{
						while (inThisArray(samesies, sameFinal, rects1[cross][i].number))
							rects1[cross][i] = initRects[(rand() % numOfRects) + 0];
					}
					if (i > 0)
					{
						for (int k = 0; k < i; k++)
						{
							while (rects1[cross][i].number == rects1[cross][k].number || inThisArray(samesies, sameFinal, rects1[cross][i].number))
							{
								rects1[cross][i] = initRects[(rand() % numOfRects) + 0];
								k = 0;
							}
						}
					}
				}
			}
		}
	}
}

bool inThisArray(int array[], int size, int check)
{
	for (int i = 0; i < size; i++)
	{
		if (check == array[i])
		{
			return true;
		}
	}
	return false;
}
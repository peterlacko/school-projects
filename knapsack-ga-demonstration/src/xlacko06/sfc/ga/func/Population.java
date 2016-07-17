/*
 * Copyright 2014 Peter Lacko
 * 
 */
package xlacko06.sfc.ga.func;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Random;

/**
 *
 * @author peterson
 */
public class Population {

	Random ra;
	/* holds values and weights of items in knapsack */
	ArrayList<Item> knapsack;
	int maxKnapsackVolume;
	/* holds population with fitness values and volumes*/
	int populationSize;
	ArrayList<Chromosome> population;
	/* parents, their number set to population/2 in initialization */
	ArrayList<Chromosome> parents;
	int noParents;
	/* offspring is coming! */
	ArrayList<Chromosome> offspring;
	/* Save simulation data */
	SimulationData simulationData;

	/*
	Constructor for new population
	*/
	public Population(ArrayList<Item> knap, int knapSize) {
		ra = new Random();
		knapsack = knap;
		maxKnapsackVolume = knapSize;
		offspring = new ArrayList<>();
		population = new ArrayList<>();
		simulationData = new SimulationData();
	}

	
	/*
	Called only once. Initializes population to random start values
	and computes initial volumes and fitness values of each knapsack.
	@param size of population
	*/
	public void initPopulation(int pSize) {
		populationSize = pSize;
		/* initialize population */
		byte[] tempPopulation;
		tempPopulation = new byte[pSize];
		ra.nextBytes(tempPopulation);
		for(int i = 0; i < pSize; i++) {
			Chromosome c = new Chromosome();
			c.setChromosome(tempPopulation[i]);
			population.add(c);
		}


		noParents = population.size()/2 > 2 ? population.size()/2:2;

		/* compute fitness values for initial population */
		computeFitnessValues(population);
		
	}

	public void selectParents(Types.ParentSelection selectionType) {
		simulationData.pselType = selectionType;
		switch (selectionType) {
			case ROULETTE:
				rouletteSelection();
				break;
			case ELITE:
				eliteSelection();
				break;
			case TOURNAMENT:
				tournamentSelection();
				break;
			default:
				rouletteSelection();
		}
	}

	public void performCrossover(Types.Recombination recombinationType) {
		switch (recombinationType) {
			case ONE:
				performCrossover(1);
				break;
			case TWO:
				performCrossover(2);
				break;
			case THREE:
				performCrossover(3);
				break;
			case UNIFORM:
				performUniformCrossover();
				break;
		}
	}

	/*
	Performs crossover of certain bits between two random parents.
	@param number of points for crossover
	*/
	private void performCrossover(int noPoints) {
		ArrayList<Integer> points = new ArrayList(Arrays.asList(1,2,3,4,5,6,7));
		Collections.shuffle(points);
		points.subList(noPoints, points.size()).clear();
		Collections.sort(points);
		points.add(8); // for easier manipulation
		byte pattern = giveMePattern(points);
		/* select 2 random parents for each chromosome and do it! */
		for (int i = 0; i < populationSize; i++) {
			int r1 = ra.nextInt(noParents);
			int r2 = ra.nextInt(noParents);
			Chromosome os = new Chromosome();
			os.setChromosome((byte) ((pattern & parents.get(r1).getChromosome())
				| (~pattern & parents.get(r2).getChromosome())));
			offspring.add(os);
			points.remove(new Integer(8));
			simulationData.recOffspring.add(
				new SimulationData.child(
				os,
				new Chromosome(parents.get(r1)),
				new Chromosome(parents.get(r2)),
				new ArrayList(points))
			);

		}
		computeFitnessValues(offspring);
		for (int o = 0; o < offspring.size(); o++) {
			simulationData.recOffspring.get(o).setChild(offspring.get(o));
		}

	}

	/*
	Performs uniform crossover between two parents, ie. each bit is selected
	with 50% probability from one of 2 parents.
	*/
	private void performUniformCrossover() {
		for (int i = 0; i < populationSize; i++) {
			byte pattern = giveMePattern();
			int r1 = ra.nextInt(noParents);
			int r2 = ra.nextInt(noParents);
			Chromosome os = new Chromosome();
			os.setChromosome((byte) ((pattern & parents.get(r1).getChromosome())
				| (~pattern & parents.get(r2).getChromosome())));
			offspring.add(os);
			simulationData.recOffspring.add(new SimulationData.child(
				os,
				new Chromosome(parents.get(r1)),
				new Chromosome(parents.get(r2)),
				new ArrayList(Arrays.asList(1,2,3,4,5,6,7,8))));
		}
		computeFitnessValues(offspring);
	}

	/*
	Perform mutation in random chromosome
	For speeding up computation, only such a random bit is is selected
	which doesn't change volume over maxKnapsackVolume.
	*/
	public void performMutation() {
		int r = ra.nextInt(offspring.size());
		ArrayList<Integer> rg = new ArrayList(Arrays.asList(0,1,2,3,4,5,6,7));
		Collections.shuffle(rg);
		for (int i:rg) {
			byte val = (byte) (offspring.get(r).getChromosome());
			if ((val & (1<<i)) == 0) {
				val = (byte)(offspring.get(r).getChromosome() | (byte)(1<<i));
			} else {
				val = (byte)(offspring.get(r).getChromosome() & ~(1<<i));
			}
			// compute volume
			int currVolume = 0;
			for (int j = 0; j < 8; j++) {
				byte isIn;
				if ((1<<(j) & val) == 0) {
					isIn = 0;
				} else {
					isIn = 1;
				}
				currVolume += isIn*(128+(int)knapsack.get(j).getVolume());
			}
			// if volume is *OK*, compute fitness and set both,
			// volume and fitness to new values
			if (currVolume <= maxKnapsackVolume) {
				int currValue = 0;
				for (int j = 0; j < 8; j++) {
					byte isIn;
					if ((1<<(j) & val) == 0) {
						isIn = 0;
					} else {
						isIn = 1;
					}
					currValue += isIn*(128+(int)knapsack.get(j).getValue());
				}
				simulationData.mutChromosomeToMutate = new Chromosome(offspring.get(r));
				simulationData.mutParents = copyChromosomeArray(offspring);
				offspring.get(r).setChromosome(val);
				offspring.get(r).setVolume(currVolume);
				offspring.get(r).setFitnessValue(currValue);
				simulationData.mutMutatedChromosome = new Chromosome(offspring.get(r));
				simulationData.mutBitChanged = (byte)i;
				simulationData.mutOffspring = new ArrayList(offspring);
				break;
			}
		}
	}

	public ArrayList getKnapsack() {
		return knapsack;
	}


	/*
	Compute fitness values of given population.
	For getting quicker solution, chromosomes with volume bigger than
	maxKnapsackVolume are modified to meet this criterion.
	*/
	private void computeFitnessValues(ArrayList<Chromosome> pop) {
		ArrayList<Double> fitness= new ArrayList<>(pop.size());
		for (int i = 0; i < pop.size(); i++) {
			ArrayList<Byte> aux = new ArrayList<>(Arrays.asList(
				(byte)(0), (byte)(1), (byte)(2), (byte)(3),
				(byte)(4), (byte)(5), (byte)(6),(byte)(7)));

			/* first calculate volume of all items */
			int currVolume = 0;
			for (int j = 0; j < 8; j++) {
				byte isIn;
				if ((1<<(j) & pop.get(i).getChromosome()) == 0) {
					isIn = 0;
				} else {
					isIn = 1;
				}
				currVolume += isIn*(128+(int)knapsack.get(j).getVolume());
			}
			/* if volume of all items is bigger than knapsack,
			 * invert random '1' bit to 0 and check. */
			int k = 0;
			while (currVolume > maxKnapsackVolume && k < 8) {
				int ran = ra.nextInt(8-k);
				byte val = (byte) (pop.get(i).getChromosome() & ~(1<<aux.get(ran)));
				if (val != pop.get(i).getChromosome()) {
					pop.get(i).setChromosome(val);
					currVolume -= (128+(int)knapsack.get(aux.get(ran)).getVolume());
				}
				aux.remove(ran);
				++k;
			}
			pop.get(i).setVolume((double) currVolume);
			/* now compute fitness value, ie real value of items
			 * in knapsack */
			int currValue = 0;
			for (int j = 0; j < 8; j++) {
				byte isIn;
				if ((1<<(j) & pop.get(i).getChromosome()) == 0) {
					isIn = 0;
				} else {
					isIn = 1;
				}
				currValue += isIn*(128+(int)knapsack.get(j).getValue());
			}
			pop.get(i).setFitnessValue((double) currValue);
		}
	}

	ArrayList getPopulation() {
		return population;
	}

	/*
	* select half of population for reproduction using roulete wheel
	* @return new parents
	*/
	private void rouletteSelection() {
		parents = new ArrayList<>();
		double sumOfValues = 0;
		for (Chromosome i:population) {
			sumOfValues += i.getFitnessValue();
		}
		ArrayList<Double> valuesByProportions = new ArrayList<>();
		for (int i = 0; i < population.size(); i++) {
			valuesByProportions.add(population.get(i).getFitnessValue()/sumOfValues);
		}
		simulationData.pselRselValByProportion = new ArrayList(valuesByProportions);
		for (int i = 1; i < valuesByProportions.size(); i++) {
			valuesByProportions.set(i,
				valuesByProportions.get(i-1)+valuesByProportions.get(i));
		}
		simulationData.pselRselCumValByProportion = new ArrayList(valuesByProportions);
		for (int i = 0; i < noParents; i++) {
			double rn = ra.nextDouble();
			simulationData.pselRselRandomNumbers.add(rn);
			for (int j = 0; j < valuesByProportions.size(); j++) {
				if (rn < valuesByProportions.get(j)) {
					parents.add(population.get(j));
					break;
				}
			}
		}
		//Collections.sort(simulationData.pselRselRandomNumbers);
		simulationData.oldPopulation = copyChromosomeArray(population);
		simulationData.pselParents = simulationData.recParents = copyChromosomeArray(parents);
	}

	/* select best half of population for later reproduction
	* @return parents
	*/
	private void eliteSelection() {
		parents = new ArrayList<>(population);
		Collections.sort(parents);
		parents.subList(noParents, parents.size()).clear();
		simulationData.oldPopulation = copyChromosomeArray(population);
		simulationData.pselParents = simulationData.recParents = copyChromosomeArray(parents);
	}

	/* in each step randomly select certain number of chromosomes,
	* pick the one with best fit value, and continue while number of
	* desired parents is not matched
	*/
	private void tournamentSelection() {
		parents = new ArrayList<>();
		for(int i = 0; i < noParents; i++) {
			ArrayList<Chromosome> candidates = new ArrayList<>(population);
			Collections.shuffle(candidates);
			candidates.subList(noParents, candidates.size()).clear();
			Collections.sort(candidates);
			simulationData.pselTsel.add(candidates);
			parents.add(candidates.get(0));
		}
		simulationData.oldPopulation = copyChromosomeArray(population);
		simulationData.pselParents = simulationData.recParents = copyChromosomeArray(parents);
	}

	/*
	Creates 0/1 bit pattern used for crossover.
	@param list of points where to perform crossover
	*/
	private byte giveMePattern(ArrayList<Integer> points) {
		byte b = 0;
		int currShift = 0;
		boolean turn = true;
		for (byte p = 0; p < points.size(); p++) {
			for(int i = currShift; i<points.get(p); i+=1) {
				if (turn) {
					b = (byte)(b | (byte)1 << i);
				} else {
					b = (byte)(b | (byte)0 << i);
				}
			}
			currShift = points.get(p);
			turn = !turn;
		}
		return b;
	}

	private byte giveMePattern() {
		byte b = 0;
		for (int i = 0; i < 8; i++) {
			float r = ra.nextFloat();
			if (r < 0.5) {
				b = (byte)(b | (byte)1 << i);
	 		} else {
				b = (byte) (b | (byte)0 << i);
			}
		}
		return b;
	}

	/*
	Creates new population by inserting 'n' best chromosomes into old population
	*/
	void performReinsertion(int n) {
		Collections.sort(population);
		simulationData.reinSortedPopulation = copyChromosomeArray(population);
		Collections.reverse(population);
		Collections.sort(offspring);
		for (int i = 0; i < n; i++) {
			population.set(i, offspring.get(i));
		}

		simulationData.reinNewPopulation = copyChromosomeArray(population);
	}

	/*
	Cleans population from old chromosomes and defected youth.
	*/
	SimulationData cycleEnd() {
		offspring = new ArrayList<>();
		parents = new ArrayList<>();
		SimulationData ret = simulationData;
		simulationData = new SimulationData();
		return ret;
	}

	private ArrayList<Chromosome> copyChromosomeArray(ArrayList<Chromosome> old) {
		ArrayList<Chromosome> ret = new ArrayList();
		for (Chromosome c: old) {
			ret.add(new Chromosome(c));
		}
		return ret;
	}
}
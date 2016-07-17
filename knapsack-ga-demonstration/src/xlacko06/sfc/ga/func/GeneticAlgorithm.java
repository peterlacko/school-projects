/*
 * Copyright 2014 Peter Lacko
 */
package xlacko06.sfc.ga.func;

import java.util.ArrayList;

/**
 *
 * @author peterson
 */
public class GeneticAlgorithm {
	private Population population;
	int populationSize;
	int reproductionCycles;
	int cyclesPerformed;
	Types.ParentSelection parentSelection;
	Types.Recombination recombinationType;
	int reinsertionNumber;
	public ArrayList<SimulationData> simulationData;

	public GeneticAlgorithm() {
		simulationData = new ArrayList<>();
	}

	/*
	* performs one cycle of genetic algorithm
	*/
	public void performCycle() {
		if (!simulationComplete()) {
			population.selectParents(parentSelection);
			population.performCrossover(recombinationType);
			population.performMutation();
			population.performReinsertion(reinsertionNumber);
			simulationData.add(population.cycleEnd());
			cyclesPerformed++;
		} else {
			System.out.println("Simulation is over");
		}
	}

	/*
	* Method is called only once for creating first population
	* @return initial population
	*/
	public Population initPopulation(ArrayList<Item> knap, int knapVol) {
		population = new Population(knap, knapVol);
		population.initPopulation(populationSize);
		return population;
	}

	Population formNewPopulation() {
		return population;
	}

	public void setPopulationSize(int value) {
		populationSize = value;
	}

	public void setReproductionCycles(int value) {
		reproductionCycles = value;
		cyclesPerformed = 0;
	}

	public void setParentSelection(int val) {
		switch (val) {
			case 0: parentSelection = Types.ParentSelection.ROULETTE;
				break;
			case 1: parentSelection = Types.ParentSelection.ELITE;
				break;
			case 2: parentSelection = Types.ParentSelection.TOURNAMENT;
				break;
			default: parentSelection = Types.ParentSelection.ROULETTE;
		}
	}

	public void setRecombinationType(int val) {
		switch (val) {
			case 0: recombinationType = Types.Recombination.ONE;
				break;
			case 1: recombinationType = Types.Recombination.TWO;
				break;
			case 2: recombinationType = Types.Recombination.THREE;
				break;
			case 3: recombinationType = Types.Recombination.UNIFORM;
				break;
			default: recombinationType = Types.Recombination.ONE; 
		}
	}

	public void setReinsertion(int value) {
		if (value > populationSize)
			reinsertionNumber = populationSize;
		else
			reinsertionNumber = value;
	}

	public boolean simulationComplete() {
		return cyclesPerformed == reproductionCycles;
	}

	public int cyclesPerformed() {
		return cyclesPerformed;
	}

	public int getPopulationSize() {
		return populationSize;
	}

	public int getNoParents() {
		return populationSize/2 > 2 ? populationSize/2:2;
	}

	public int getReinsertionNumber() {
		return reinsertionNumber;
	}
}

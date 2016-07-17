/*
 * Copyright 2014 Peter Lacko
 * 
 */
package xlacko06.sfc.ga.func;

import java.util.ArrayList;

/**
 *
 * @author peterson
 */
public class SimulationData {
	/*
	Data for parent selection.
	*/
	public Types.ParentSelection pselType;
	public ArrayList<Chromosome> oldPopulation;
	public ArrayList<Chromosome> pselParents;
	// roulette wheel selection
	public ArrayList<Double> pselRselValByProportion;
	public ArrayList<Double> pselRselCumValByProportion;
	public ArrayList<Double> pselRselRandomNumbers;
	// elite selection
	// nothing here
	// tournament selection
	public ArrayList<ArrayList<Chromosome>> pselTsel;

	/*
	Data for recombination
	*/
	public ArrayList<Chromosome> recParents;
	public ArrayList<child> recOffspring;

	/*
	Data for mutation
	*/
	public ArrayList<Chromosome> mutParents;
	public Chromosome mutChromosomeToMutate; // one of children
	public byte mutBitChanged;
	public Chromosome mutMutatedChromosome;
	public ArrayList<Chromosome> mutOffspring;

	/*
	Data for reinsertion
	*/
	public ArrayList<Chromosome> reinSortedPopulation;
	public ArrayList<Chromosome> reinNewPopulation;
	
	public SimulationData() {
		oldPopulation = new ArrayList<>();
		pselParents = new ArrayList<>();
		pselRselValByProportion = new ArrayList<>();
		pselRselCumValByProportion = new ArrayList<>();
		pselRselRandomNumbers = new ArrayList<>();
		pselTsel = new ArrayList<>(); // TODO initialize arrays inside list
		recOffspring = new ArrayList<>(); // TODO initialize array insideo
		mutParents = new ArrayList<>(); // TODO create from recOffspring
		mutChromosomeToMutate = new Chromosome();
		mutMutatedChromosome = new Chromosome(); // necessary?
	}

	static public class child {
		public Chromosome child;
		public Chromosome p1;
		public Chromosome p2;
		ArrayList<Integer> crossPoints;

		public child (Chromosome c, Chromosome pp1, Chromosome pp2, ArrayList<Integer> cp) {
			child = c;
			p1 = new Chromosome(pp1);
			p2 = new Chromosome(pp2);
			crossPoints = new ArrayList(cp);
		}

		@Override
		public String toString() {
			return child.toString() + p1.toString() + p2.toString() + "\n";
		}

		public void setChild(Chromosome c) {
			child = new Chromosome();
			child.setChromosome(c.getChromosome());
			child.setFitnessValue(c.getFitnessValue());
			child.setVolume(c.getVolume());
		}

		public Chromosome getChild() {
			return child;
		}

		public Chromosome getParent1() {
			return p1;
		}

		public Chromosome getParent2() {
			return p2;
		}

		public ArrayList<Integer> getCrossPoints() {
			return crossPoints;
		}
	}
}

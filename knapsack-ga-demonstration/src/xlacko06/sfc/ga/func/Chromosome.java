/*
 * Copyright 2014 Peter Lacko
 * 
 */
package xlacko06.sfc.ga.func;

/**
 *
 * @author peterson
 */
public class Chromosome implements Comparable<Chromosome> {
	private byte c;
	private double f;
	private double volume;

	public Chromosome(Chromosome get) {
		c = get.c;
		f = get.f;
		volume = get.volume;
	}

	public Chromosome() {
	}

	public void setChromosome(byte b) {
		c = b;
	}

	public void setFitnessValue(double d) {
		f = d;
	}

	public void setVolume(double d) {
		volume = d;
	}

	public byte getChromosome() {
		return c;
	}

	public double getFitnessValue() {
		return f;
	}

	public double getVolume() {
		return volume;
	}

	@Override
	public String toString() {
		return "Chromosome:  volume, fitness_value\n" + Byte.toString(c) + ": " + Double.toString(volume)
			+ ", " + Double.toString(f) + "\n";
	}

	@Override
	public int compareTo(Chromosome o) {
		double compareValue = (o.getFitnessValue());
		if((this.f - compareValue) < 0)
			return 1;
		else if ((this.f - compareValue) > 0)
			return -1;
		else
			return 0;
	}
}

/*
 * Copyright 2014 Peter Lacko
 * 
 */
package xlacko06.sfc.ga.func;

import java.util.Random;

/**
 * 
 * @author peterson
 */
public class Item{
	byte[] values;
	Random ra;

	public Item() {
		ra = new Random();
		values = new byte[2];
		ra.nextBytes(values);
	}

	public byte getValue() {
		return values[0];
	}

	public byte getVolume() {
		return values[1];
	}

	public void setValue(byte value) {
		this.values[0] = value;
	}

	public void setVolume(byte weight) {
		this.values[1] = weight;
	}

	@Override
	public String toString() {
		return "Volume: " + Integer.toString(128+(int)values[1]) + " ,"
			+ "Value: " + Integer.toString(128+(int)values[0]);
	}

}

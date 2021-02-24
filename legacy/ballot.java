// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;
import java.util.TreeSet;


public class ballot {
	

	static String csvFile = "/Users/mleming/Desktop/Cambridge/churchill_mcr/room_ballot/people2019.csv"; //TODO: change these!
	static String csvRooms = "/Users/mleming/Desktop/Cambridge/churchill_mcr/room_ballot/rooms2019.csv"; 
	
	private static class Room implements Comparable<Room>{
		String name;
		int price;
		@SuppressWarnings("unused")
		boolean doubleBed;
		boolean outer;
		
		public Room(String n, int p, boolean db, boolean o) {
			name = n;
			price = p;
			doubleBed = db;
			outer = o;
			if(o) {outerHostels++;
			System.out.println(n + " is outer");
			}
			
		}

		@Override
		public int compareTo(Room o) {
			return name.compareTo(o.name);
		}
	}
	
	private static class Person implements Comparable<Person>{
		String crsid;
		String name;
		Room allocatedRoom;
		long randomPriority;
		int priority;
		int choice = 0;
		int terms;
		@Override
		public int compareTo(Person o) {
			
			if(priority > o.priority) return 1;
			if(priority < o.priority) return -1;
			
			if(terms > o.terms) return 1;
			if(terms < o.terms) return -1;
			
			if(randomPriority > o.randomPriority) return 1;
			if(randomPriority < o.randomPriority) return -1;
			
			return 0;
		}
		
		List<Room> desiredRooms;
		
		public Person(String crs, List<Room> dr, int p, int t, String n) {
			
			crsid = crs;
			desiredRooms = dr;
			priority = p;
			terms = t;
			name = n;
			
			Calendar now = Calendar.getInstance();
			long year = now.get(Calendar.YEAR);
			Random r = new Random(crs.hashCode() + year); //means priorities don't stay same each year
			
			randomPriority = r.nextLong(); //deterministic is best. BEST.
			
			
			
		}
	}
	
	static Map<String,Room> roomMap;
	

	
	static Set<Room> freeRooms;
	
	static ArrayList<Person> people;
	
	static ArrayList<Room> rooms;
	
	static int outerHostels = 0;
	static int totalOuter = 0;
	
	public static void main(String args[]) {
		
		freeRooms = new TreeSet<ballot.Room>();
		
		roomMap = new HashMap<String, ballot.Room>();
		
		people = new ArrayList<ballot.Person>();
		rooms = new ArrayList<ballot.Room>();
		
		
		int allocatedRooms = 0;
		
		int roomTarget = 58;
		
		int choices = 10;
		

		BufferedReader br = null;
		BufferedReader br2 = null;
		String line = "";
		String cvsSplitBy = ",";
		
		try {
			br = new BufferedReader(new FileReader(csvRooms));
		 
			while ((line = br.readLine()) != null) {

			        // use comma as separator
				String[] room = line.split(cvsSplitBy);
				
				System.out.println(room[1]);
				
				
				
				boolean outer = room.length>=8 && room[7].equals("Y");

				Room r = new Room(room[1], Integer.parseInt(room[2]), false, outer);

				freeRooms.add(r);
				rooms.add(r);
				roomMap.put(r.name,r);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		finally {
			if (br != null) {
				try {
					br.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		
		

		try {
			br2 = new BufferedReader(new FileReader(csvFile));
		 
			while ((line = br2.readLine()) != null) {
				

			        // use comma as separator
				String[] person = line.split(cvsSplitBy);
				
				int size = person.length;
				
				ArrayList<Room> dr = new ArrayList<ballot.Room>(choices);
				
				for(int x=8; x< 8+choices; x++) {
					dr.add(roomMap.get(person[x]));
					if(roomMap.get(person[x]) == null) System.out.println(person[x] + "not in ballot");
				}
				
				System.out.println(person[1]);

				Person p = new Person(person[1],dr,Integer.parseInt(person[3]),Integer.parseInt(person[4]),person[0]);
				people.add(p);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		finally {
			if (br2 != null) {
				try {
					br2.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		
		boolean removed = false;
		
		
		//r = new Random(13372016);
		
		Collections.sort(people);
		
		totalOuter = outerHostels;
		
		int firstInGroup =0;
		int currentPriority = 1;
		int currentTime = 0;
		
		for(int x=0; x< people.size(); x++) {
			Person p = people.get(x);
			for(int y =0; y< p.desiredRooms.size(); y++) {
				Room r = p.desiredRooms.get(y);
				if(r!=null && freeRooms.contains(r)) {
					p.allocatedRoom = r;
					freeRooms.remove(r);
					allocatedRooms++;
					if(r.outer) outerHostels--;
					p.choice = y+1;
					break;
				}
			}
			if(p.allocatedRoom == null) {
				int pricediff = Integer.MAX_VALUE;
				
				for(Room r : rooms) {
					int priceDiff = Math.abs(r.price - p.desiredRooms.get(0).price);
					if(priceDiff < pricediff && freeRooms.contains(r)) {
						pricediff = priceDiff;
						p.allocatedRoom = r;
						p.choice = 11;
						
					}
				}
				
				freeRooms.remove(p.allocatedRoom);
				allocatedRooms++;
				if(p.allocatedRoom.outer) outerHostels--;
			}
			

			
			
			if(roomTarget - outerHostels == allocatedRooms && !removed) {
				firstInGroup = x;
				System.out.println("Removing non-outer hostels: " + outerHostels + " left out of " + totalOuter );
				for(Room r : rooms) {
					if(!r.outer) {freeRooms.remove(r);
					
					}
				}
				assert(freeRooms.size() == outerHostels);
				removed = true;
			}
			if((allocatedRooms > roomTarget) ) {
				//limited simulated annealing step!
				System.out.println("Annealing from " + firstInGroup + " to " + (x));
				currentPriority = p.priority; currentTime = p.terms;
				Calendar now = Calendar.getInstance();
			long year = now.get(Calendar.YEAR) ;
				Random rand = new Random(year);
				int person1 = rand.nextInt(x-firstInGroup) + firstInGroup;
				 
				double maxTemp = 600000;
				
				for(int temp = 1; temp < maxTemp; temp++) {
					int person2 = rand.nextInt(x-firstInGroup+1) + firstInGroup;
					
					Person p1 = people.get(person1);
					Person p2 = people.get(person2);
					
					int newChoice1 = 11;
					int newChoice2 = 11;
					for(int z=9; z>=0; z--) {
						if(p2.desiredRooms.get(z) == p1.allocatedRoom) newChoice2 = z+1;
						if(p1.desiredRooms.get(z) == p2.allocatedRoom) newChoice1 = z+1;
					}
					
					double e = p1.choice + p2.choice;
					double eprime = newChoice1 + newChoice2;
					
					if((eprime < e) /*|| ((Math.exp((e - eprime) / ((double) temp / maxTemp)) >= rand.nextDouble()))*/) {
						//I've taken out the random bit for now: we're only using it two swap outer hostels, so it should always be the choice of n vs 11!
						p1.choice = newChoice1;
						p2.choice = newChoice2;
						Room t = p1.allocatedRoom;
						p1.allocatedRoom = p2.allocatedRoom;
						p2.allocatedRoom = t;
						
						if(p1 != p2) System.out.println("Swapping " + p1.crsid + " and " + p2.crsid);
					} //else System.out.println("Not Swapping " + p1.crsid + " and " + p2.crsid);
					
					person1 = person2;
				}
				firstInGroup = x; 
			}
			
			if(allocatedRooms >= roomTarget) { System.out.println("Target Reached");break;}
		}
		
		assert(allocatedRooms == roomTarget);
		
		int totalCost = 0;
		for(Person p : people) {
			System.out.println(p.name + " , " + p.crsid + " , " + (p.allocatedRoom == null ? "" : p.allocatedRoom.name) + ", (choice " + p.choice + ")");

			totalCost+= p.choice;
		}
		
		System.out.println("Choice sum: " + totalCost);

	}

}

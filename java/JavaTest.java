public class JavaTest {
	public JavaTest() {
		System.out.println("Creating the JavaTest class...");
	}

	public void iterate() {
		System.out.println("Iterating java object.");
	}

	public int intTest(int n) {
		System.out.println("The JavaTest int input was " + n);
		System.out.println("returning n + n = " + (n + n));
		return n + n;
	}

	public double doubleTest(double n) {
		System.out.println("The JavaTest double input was " + n);
		System.out.println("returning n + n = " + (n + n));
		return n + n;
	}

	public String stringTest(String n) {
		System.out.println("The JavaTest string input was " + n);
		System.out.println("returning n + n = " + (n + n));
		return n + n;
	}
}

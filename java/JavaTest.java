public class JavaTest {
    public JavaTest() {
        System.out.println("Creating the test class...");
	}

	public void iterate() {
        System.out.println("Iterating java objecct.");
	}

	public int intTest(int n) {
        System.out.println("The int input was " + n);
		return n + n;
	}

	public double doubleTest(double n) {
        System.out.println("The double input was " + n);
		return n + n;
	}

	public String stringTest(String n) {
        System.out.println("The string input was " + n);
		return n + n;
	}
}

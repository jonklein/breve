import java.lang.Object;
import java.lang.Class;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.NoSuchMethodException;
import java.lang.reflect.Array;

public class MethodFinder {
	Class methodClass = java.lang.reflect.Method.class;
	Class voidClass = java.lang.Void.TYPE;

	public Object findMethod(Class type, String name, int arguments, char types[]) throws NoSuchMethodException {
		Method methods[], method;
		String result;

		if(name.equals("<init>")) return this.findConstructor(type, arguments);

		try {
			methods = type.getMethods();
		} catch (Exception e) {
			System.out.println("Cannot get methods");
			throw new NoSuchMethodException();
		}

		for(int i=0;i<methods.length;i++) {
			int paramCount;

			method = methods[i];

			if(name.equals(method.getName())) {
				paramCount = method.getParameterTypes().length;

				if (paramCount == arguments) return method;
			}
		}

		System.out.println("Method \"" + name + "\" not found");

		throw new NoSuchMethodException();
	}
	
	public Constructor findConstructor(Class type, int arguments) throws NoSuchMethodException {
		Constructor constructors[], constructor;
		String result;

		try {
			constructors = type.getConstructors();
		} catch (Exception e) {
			System.out.println("Cannot get constructors");
			throw new NoSuchMethodException();
		}

		for(int i=0;i<constructors.length;i++) {
			constructor = constructors[i];

			int paramCount = constructor.getParameterTypes().length;

			if (paramCount == arguments) return constructor;
		}

		throw new NoSuchMethodException();
	}

	public String getSignature(Object o) {
		String result;

		Class returnType;
		Class params[];

		if(o.getClass() == methodClass) {
			returnType = ((Method)o).getReturnType();
			params = ((Method)o).getParameterTypes();
		} else {
			returnType = voidClass;
			params = ((Constructor)o).getParameterTypes();
		}

		result = "(";

		for(int n=0;n<params.length;n++) {
			result = result + this.signatureStringForClass(params[n]);
		}

		result += ")" + this.signatureStringForClass(returnType);

		return result;
	}
	
	public String getSignature(Constructor constructor) {
		String result;

		Class params[];

		params = constructor.getParameterTypes();

		result = "(";

		for(int m=0;m<params.length;m++) {
			result = result + this.signatureStringForClass(params[m]);
		}

		result += ")V";

		return result;
	}

	public char getReturnType(Object o) {
		Class returnType;

		if(o.getClass() == methodClass) returnType = ((Method)o).getReturnType();
		else returnType = voidClass;

		return this.breveTypeCharForClass(returnType);
	}

	public int getArgumentCount(Object o) {
		Class params[];

		if(o.getClass() == methodClass) params = ((Method)o).getParameterTypes();
		else params = ((Constructor)o).getParameterTypes();

		return Array.getLength(params);
	}

	public char getArgumentTypeAtIndex(Object o, int index) {
		Class params[];

		if(o.getClass() == methodClass) params = ((Method)o).getParameterTypes();
		else params = ((Constructor)o).getParameterTypes();

		return this.breveTypeCharForClass(params[index]);
	}

	public String getClassName(Object theObject) {
		return theObject.getClass().getName();
	}

	//
	// returns a char representing a Java type that breve can understand.
	// these are the same as the standard JNI types, with the exception 
	// that objects are just called 'O', instead of the full class name.
	// breve does not do object type checking for bridged objects.
	//

	public char breveTypeCharForClass(Class theClass) {
		String name;

		name = theClass.getName();

		if(name.equals("int")) {
			return 'I';
		}
		if(name.equals("long")) {
			return 'J';
		}
		if(name.equals("double")) {
			return 'D';
		}
		if(name.equals("float")) {
			return 'F';
		}
		if(name.equals("short")) {
			return 'S';
		}
		if(name.equals("byte")) {
			return 'B';
		}
		if(name.equals("char")) {
			return 'C';
		}
		if(name.equals("boolean")) {
			return 'Z';
		}
		if(name.equals("void")) {
			return 'V';
		}
		if(name.equals("java.lang.String")) {
			return 'T';
		}

		return 'O';
	}

	public String signatureStringForClass(Class theClass) {
		String name;

		name = theClass.getName();

		if(name.equals("int")) {
			return "I";
		}
		if(name.equals("long")) {
			return "J";
		}
		if(name.equals("double")) {
			return "D";
		}
		if(name.equals("float")) {
			return "F";
		}
		if(name.equals("short")) {
			return "S";
		}
		if(name.equals("byte")) {
			return "B";
		}
		if(name.equals("char")) {
			return "C";
		}
		if(name.equals("boolean")) {
			return "Z";
		}
		if(name.equals("void") || name.equals("java.lang.Void")) {
			return "V";
		}

		return "L" + name.replace('.', '/') + ";";
	}
}

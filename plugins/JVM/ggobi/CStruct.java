package ggobi;

public class CStruct {
    double address;
    public CStruct(double val) {
	address = val;
    }

    public CStruct() {
    }

    public double getAddress() {
	return(address);
    }
    public void setAddress(double val) {
	address = val;
    }

    public String toString() {
	return("<"+getClass().getName() + " @" + (long)address +">");
    }
}

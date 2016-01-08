import javax.swing.text.html.HTMLDocument;
import java.lang.Iterable;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.NoSuchElementException;

//This method used a check-and-resize array to impelemnt a stack
//Good memory performance

public class Stacks<T> implements Iterable<T>{  //use a array list to implement
    private T[] q;     //element typed array
    private int N;     //Size



    public Stacks(){
        q = (T[]) new Object[2];  //Use the base class to initialize first and last
        N = 0;

    }

    public boolean isEmpty() {
        return N == 0;
    }

    public int size(){
        return N;
    }
    private void resize(int capacity) {
        assert capacity >= N;
        T[] temp = (T[]) new Object[capacity];
        for (int i = 0; i < N; i++) {
            temp[i] = q[i];
        }
        q = temp;
    }
    public void push(T data){

        if (N == q.length) resize(2*q.length); // if index out of 2 or the size
        q[N++] = data;

    }
    public T pop(){
        if (isEmpty()) throw new NoSuchElementException("Stack underflow");
        else {
        T temp = q[N-1];
            q[N-1] = null; //Safety against floating pointer

        N -=1 ;
            if (N>0 && N==q.length/4) resize(q.length/2);  //shrink size to avoid memory leak
            return temp;
        }


    }
    public T peek() {
        if (isEmpty()) throw new NoSuchElementException("Stack underflow");
        return q[N-1];
    }

    public Iterator<T> iterator(){
        //this method allow data stru to be iterated in java fashion
        return new ListIterator();
    }
    private class ListIterator implements Iterator<T> {

        private int i;

        public void ListIterator(){
            i = 0;
        }
        public boolean hasNext() {

            return i==N-1;
        }
        public void remove(){

            throw new UnsupportedOperationException();
        }
        public T next() {

            if (!hasNext()) throw new NoSuchElementException();

            else {
                i ++;
                return q[i];
            }
        }

    }

    public static void main(String[] args) {
        Stacks<String> s = new Stacks<String>();
        while (!StdIn.isEmpty()) {
            String item = StdIn.readString();
            if (!item.equals("-")) s.push(item);
            else if (!s.isEmpty()) StdOut.print(s.pop() + " ");
        }
        StdOut.println("(" + s.size() + " left on stack)");
        while (!s.isEmpty()) {

            StdOut.println(" " + s.pop());
        }
    }


}
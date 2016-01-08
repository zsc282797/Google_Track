import javax.swing.text.html.HTMLDocument;
import java.lang.Iterable;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.NoSuchElementException;

//This is a implementation using a linked list sort of stype to
//complete the ADT function as a Bag

public class Bag<Item> implements Iterable<Item>{

    private Node<Item> first;
    private int N;

    private static class Node<Item>{
        private Item item;
        private Node<Item> next;

    }

    public Bag() {
        first = null;
        N = 0;
    } //Zero instantiator

    public boolean isEmpty(){
        return first == null;
    }

    public int size(){
        return N;
    }
    public void add(Item item){

        Node<Item> oldfirst = first;
        first = new Node<Item>();
        first.item = item;
        first.next = oldfirst;
        N++;  //By doing this method the newly added item is always the first
    }
    public Iterator<Item> iterator(){
        //this method allow data stru to be iterated in java fashion
        return new ListIterator<Item>(first);


    }

    private class ListIterator<Item> implements Iterator<Item>{
        private Node<Item> current;

        public ListIterator(Node<Item> first){
            current = first;
        }

        public boolean hasNext() {return current!= null;}
        public void remove() {throw new UnsupportedOperationException(); }

        public Item next() {
            if (!hasNext()) throw new NoSuchElementException();
            Item item = current.item; //get item from the current node
            current = current.next;   //let the flag moves to the next one
            return item;              //return the holding item
        }
    }

    public static void main(String[] args) { //Unittest assumign that the input item is String
        Bag<String> bag = new Bag<String>();
        while (!StdIn.isEmpty()) {

            String item = StdIn.readString();
            bag.add(item);
        }

        StdOut.println("size of bag = " + bag.size());
        for (String s: bag){

            StdOut.println(s);
        }
    }
}
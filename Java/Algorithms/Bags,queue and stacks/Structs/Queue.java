import javax.swing.text.html.HTMLDocument;
import java.lang.Iterable;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.NoSuchElementException;

public class Queue<T> implements Iterable<T>{ //Use a linked list structure to impelement a Queue

    private Node<T> first; //First node
    private Node<T> last; //last node
    private int N;

    private static class Node<T>{
        private T data;
        private Node<T> next = null;
    }

    public Queue(){ //zero instantiator that construct an empty pointer to Null
        first = null;
        last = null;
        N = 0;
    }

    public boolean isEmpty() {return first == null; } //alternative method return N == 0;

    public void enqueue(T data){

        if (first == null){
            first = new Node<T>();
            first.data = data;
            last = first; //if the queue is empty
                          //set the last node to be the first node

        }
        else {
            Node<T> new_node;
            new_node = new Node<T>();
            new_node.data = data;  //warp data in a new node
            last.next = new_node;  //connect the node after the last flag
            last = new_node; //the newly added node become the last

        }
        N += 1;        //add 1 to size

    }

    public T dequeue(){

        if (isEmpty()) throw new NoSuchElementException("Queue underflow");
        else {

            T out_data = first.data;
            first = first.next; //move the first flag one more forward safe cap is done in isEmpty()
            N -= 1;
            return out_data;
        }
    }
    public int size(){
        return N;
    }

    public Iterator<T> iterator(){
        //this method allow data stru to be iterated in java fashion
        return new ListIterator<T>(first);
    }

    private class ListIterator<T> implements Iterator<T>{
        private Node<T> current;

        public ListIterator(Node<T> first){
            current = first;
        }

        public boolean hasNext() {return current!= null;}
        public void remove() {throw new UnsupportedOperationException(); }

        public T next() {
            if (!hasNext()) throw new NoSuchElementException();
            T data = current.data; //get item from the current node
            current = current.next;   //let the flag moves to the next one
            return data;              //return the holding item
        }
    }

    public static void main(String[] args) { //Unittest assumign that the input item is String
        Queue<String> queue = new Queue<String>();
        while (!StdIn.isEmpty()) {

            String item = StdIn.readString();
            queue.enqueue(item);
        }

        StdOut.println("size of queue = " + queue.size());
        for (String s: queue ){

            StdOut.println(s);
        }
    }

}
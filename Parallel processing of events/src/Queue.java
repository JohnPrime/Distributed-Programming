
import java.util.concurrent.ArrayBlockingQueue;
import java.util.logging.Level;
import java.util.logging.Logger;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 * Coada folosita pentru a stoca evenimente. Foloseste un ArrayBlockingQueue in
 * care accesul la coada este de tip FIFO.
 *
 * @author John Prime
 */
public class Queue {

    ArrayBlockingQueue<Event> bq;

    public Queue(int dim) {
        // initializez coada cu dimensiunea fixa respectiva si cu acces de tip FIFO garantat.
        bq = new ArrayBlockingQueue<>(dim, true);
    }

    /**
     * Insereaza un eveniment la sfarsitul cozii asteptand pentru spatiu daca
     * coada e plina.
     *
     * @param event
     */
    void put(Event event) {
        try {
            bq.put(event);
        } catch (InterruptedException ex) {
            Logger.getLogger(Queue.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /**
     * Extrage un eveniment din varful cozii asteptand in cazul in care coada
     * este goala.
     *
     * @return
     */
    Event get() {
        try {
            return bq.take();
        } catch (InterruptedException ex) {
            Logger.getLogger(Queue.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }
}

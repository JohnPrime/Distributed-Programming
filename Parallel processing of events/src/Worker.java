
import java.util.logging.Level;
import java.util.logging.Logger;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 * Worker pool-ul. Un worker prelucreaza evenimentul, il adauga intr-o lista si
 * apoi revine in asteptare pentru alt eveniment.
 *
 * @author John Prime
 */
public class Worker implements Runnable {

    Queue buffer;
    private final int nrEvents;
    private final int nrGenerators;
    int id;
    static int i = 0;

    Worker(Queue buffer, int nrEvents, int nrGenerators, int id) {
        this.buffer = buffer;
        this.nrEvents = nrEvents;
        this.nrGenerators = nrGenerators;
        this.id = id;
    }

    /**
     * Metoda folosita pentru a stii cand s-au terminat de prelucrat toate
     * evenimentele. Stiu numarul de evenimente pe fisier si numarul de
     * generatori de evenimente. De fiecare data cand este prelucrat un
     * eveniment, se incrementeaza i.
     *
     * @return
     */
    int getNextI() {
        int value;
        synchronized (Worker.class) {
            value = i;
            i++;
        }
        return value;
    }

    @Override
    public void run() {
        while (true) {

            // i porneste de la 0
            int generated = getNextI();

            // daca s-au prelucrat toate evenimentele 0...NrEvents - 1, atunci pot sa opresc workerii
            if (generated >= nrGenerators * nrEvents) {
                break;
            }

            // worker-ul preia un eveniment din coada
            Event task = buffer.get();

            // in functie de tipul evenimentului, worker-ul il prelucreaza si adauga sincronizat rezultatul intr-o lista
            switch (task.getType()) {
                case "FIB":

                    synchronized (Main.Fib) {
                        Main.Fib.add(task.Fib(task.getN()));
                    }

                    break;
                case "PRIME":

                    synchronized (Main.Prime) {
                        Main.Prime.add(task.Prime(task.getN()));
                    }

                    break;
                case "SQUARE":

                    synchronized (Main.Square) {
                        Main.Square.add(task.Square(task.getN()));
                    }

                    break;
                case "FACT":

                    synchronized (Main.Fact) {
                        Main.Fact.add(task.Fact(task.getN()));
                    }

                    break;
                default:
                    break;
            }

            try {
                // pun thread-ul in asteptare, pentru concurenta acestora
                Thread.sleep(50);
            } catch (InterruptedException ex) {
                Logger.getLogger(Worker.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        System.out.println("Worker-ul " + id + " a terminat cu succes!");
    }
}


import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.logging.Level;
import java.util.logging.Logger;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author John Prime
 */
public class Main {

    // cele patru liste in care voi scrie rezultatele prelucrarii evenimentelor
    public static final ArrayList<Integer> Prime = new ArrayList<>();
    public static final ArrayList<Integer> Fact = new ArrayList<>();
    public static final ArrayList<Integer> Square = new ArrayList<>();
    public static final ArrayList<Integer> Fib = new ArrayList<>();

    public static void main(String[] args) {

        // coada folosita pentru evenimente
        Queue queue = new Queue(Integer.parseInt(args[0]));

        // numarul de generatori de evenimente(thread-uri) = numarul de fisiere
        int nrEventGenerators = args.length - 2;

        // numarul de thread-uri workeri
        int nrWorkers = 4;

        // creez si pornesc thread-urile corespunzatoare
        Thread threads[] = new Thread[nrEventGenerators + nrWorkers];

        for (int i = 0; i < nrEventGenerators; i++) {
            threads[i] = new Thread(new EventGenerator(queue, args[i + 2], i));
        }

        for (int i = nrEventGenerators; i < nrEventGenerators + nrWorkers; i++) {
            threads[i] = new Thread(new Worker(queue, Integer.parseInt(args[1]), nrEventGenerators, i - nrEventGenerators));
        }

        for (int i = 0; i < nrEventGenerators + nrWorkers; i++) {
            threads[i].start();
        }

        // astept pana "mor" toate thread-urile
        for (int i = 0; i < nrEventGenerators + nrWorkers; i++) {
            try {
                threads[i].join();
            } catch (InterruptedException ex) {
                Logger.getLogger(Main.class.getName()).log(Level.SEVERE, null, ex);
            }
        }

        // sortez listele
        Collections.sort(Prime);
        Collections.sort(Fact);
        Collections.sort(Square);
        Collections.sort(Fib);

        // scriu rezultatele in fisiere corespunzatoare
        PrintWriter primeWriter, factWriter, squareWriter, fibWriter;
        try {
            primeWriter = new PrintWriter("PRIME.out", "UTF-8");
            factWriter = new PrintWriter("FACT.out", "UTF-8");
            squareWriter = new PrintWriter("SQUARE.out", "UTF-8");
            fibWriter = new PrintWriter("FIB.out", "UTF-8");

            for (Integer prime : Prime) {
                primeWriter.println(prime);
            }

            for (Integer fact : Fact) {
                factWriter.println(fact);
            }

            for (Integer square : Square) {
                squareWriter.println(square);
            }

            for (Integer fib : Fib) {
                fibWriter.println(fib);
            }

            primeWriter.close();
            factWriter.close();
            squareWriter.close();
            fibWriter.close();
        } catch (FileNotFoundException | UnsupportedEncodingException ex) {
            Logger.getLogger(Main.class.getName()).log(Level.SEVERE, null, ex);
        }

        System.out.println("Finished correctly.");
    }
}


import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 * Generatorul de evenimente.
 *
 * @author John Prime
 */
public class EventGenerator implements Runnable {

    Queue buffer;
    private final String fileName;
    int id;

    EventGenerator(Queue buffer, String fileName, int id) {
        this.buffer = buffer;
        this.fileName = fileName;
        this.id = id;
    }

    @Override
    public void run() {
        try (BufferedReader br = new BufferedReader(new FileReader(fileName))) {

            // citesc linie cu linie din fisier
            for (String line; (line = br.readLine()) != null;) {

                // separ elementele de pe linie
                String[] splittedLine = line.split(",");
                try {
                    // pun thread-ul sa astepte
                    Thread.sleep(Integer.parseInt(splittedLine[0]));

                    // creez evenimentul
                    Event event = new Event(splittedLine[1], Integer.parseInt(splittedLine[2]));

                    // il adaug in coada
                    buffer.put(event);
                } catch (InterruptedException ex) {
                    Logger.getLogger(EventGenerator.class.getName()).log(Level.SEVERE, null, ex);
                }
            }

        } catch (IOException ex) {
            Logger.getLogger(EventGenerator.class.getName()).log(Level.SEVERE, null, ex);
        }
        System.out.println("Generatorul de evenimente " + id + " a terminat cu succes!");
    }
}

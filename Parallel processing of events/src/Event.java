
import java.util.ArrayList;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 * Evenimentul care trebuie calculat.
 *
 * @author John Prime
 */
public class Event {

    // tipul evenimentului si valoarea asociata
    private String type;
    private int N;

    public Event(String type, int N) {
        this.type = type;
        this.N = N;
    }

    public void setType(String type) {
        this.type = type;
    }

    public void setN(int N) {
        this.N = N;
    }

    public String getType() {
        return type;
    }

    public int getN() {
        return N;
    }

    /**
     * Folosesc Sieve of Eratosthenes pentru a gasi toate numerele prime mai
     * mici ca un N dat.
     *
     * @param N
     * @return cel mai mare numar prim mai mic sau egal cu N
     */
    public int Prime(int N) {

        ArrayList<Boolean> primes = new ArrayList<>();

        // initial consider toate numerele prime, mai putin 0 si 1.
        for (int i = 0; i < N + 1; i++) {
            if (i >= 2 && i <= N) {
                primes.add(true);
            } else {
                primes.add(false);
            }
        }

        for (int factor = 2; factor * factor <= N; factor++) {

            // daca factor este numar prim, multiplii sai nu sunt
            // este suficient sa iau in considerare multiplii factor, factor+1, ...,  n/factor
            if (primes.get(factor)) {

                for (int j = factor; factor * j <= N; j++) {
                    primes.set(factor * j, Boolean.FALSE);
                }
            }
        }

        // ultimul numar din lista care e prim
        return primes.lastIndexOf(true);
    }

    /**
     * Calculeaza factorialul unui numar.
     *
     * @param n
     * @return
     */
    private int calculateFactorial(int n) {
        int fact = 1;
        for (int i = 1; i <= n; i++) {
            fact *= i;
        }
        return fact;
    }

    /**
     * Calculeaza cel mai mare numar care are factorialul mai mic sau egal cu N.
     *
     * @param N
     * @return
     */
    public int Fact(int N) {
        int biggestNumber = 0;

        // cat timp nu am gasit numarul al carui factorial este <= N
        while (true) {
            int factorialNumber = calculateFactorial(biggestNumber);

            if (factorialNumber > N) {
                break;
            }
            biggestNumber++;
        }

        // este numar - 1 pentru ca in conditia de mai sus factorialul lui numar e > N, iar (numar-1)! <= N
        return biggestNumber - 1;
    }

    /**
     * Calculeaza cel mai mare numar care are patratul perfect mai mic sau egal
     * N;
     *
     * @param N
     * @return
     */
    public int Square(int N) {
        int biggestNumber = 0;

        // cat timp nu am gasit numarul al carui patrat perfect este <= N
        while (true) {
            int perfectSquare = biggestNumber * biggestNumber;

            if (perfectSquare > N) {
                break;
            }
            biggestNumber++;
        }
        return biggestNumber - 1;
    }

    /**
     * Calculeaza valoarea corespunzatoare din sirul lui Fibonacci .
     *
     * @param n
     * @return
     */
    private int calculateFibonacci(int n) {
        if (n < 2) {
            return 1;
        } else {
            int F0 = 0, F1 = 1, F = 0;

            for (int i = 2; i <= n; i++) {
                F = F0 + F1;
                F0 = F1;
                F1 = F;
            }
            return F;
        }
    }

    /**
     * Calculeaza cel mai mare numar pentru care valoarea corespunzatoare din
     * sirul lui Fibonacci este mai mica sau egala cu N.
     *
     * @param N
     * @return
     */
    public int Fib(int N) {
        int biggestNumber = 0;

        // cat timp nu am gasit numarul a carui valoare din sirul lui Fibonacci este <= N
        while (true) {
            int fibonacciNumber = calculateFibonacci(biggestNumber);

            if (fibonacciNumber > N) {
                break;
            }
            biggestNumber++;
        }
        return biggestNumber - 1;
    }
}

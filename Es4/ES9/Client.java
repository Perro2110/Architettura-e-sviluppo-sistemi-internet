import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.util.Scanner;

public class Client {
    public static void main(String[] args) {
        // java RemoteSquareClient nomehost porta
        // args[0] args[1]
        if (args.length != 2) {
            System.err.println("Errore! Sintassi corretta è:");
            System.err.println("\tjava Client nomehost porta ");
            System.exit(1);
        }

        var hostname = args[0];
        var port = args[1];

        Scanner scanner = new Scanner(System.in);
        String input;

        System.out.print("Inserisci un numero o 'fine' per terminare: ");
        input = scanner.nextLine(); // Legge la linea completa

        while (!("fine".equals(input))) {

            try {
                var s = new Socket(hostname, Integer.parseInt(port));
                var netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8")); // ESTREMI input
                                                                                                    // SOKET
                var netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8")); // ESTREMI output
                                                                                                       // SOKET
                String line;

                netOut.write(input);
                netOut.newLine();
                netOut.flush(); // non necessario; performance optimization

                // RICEVO IN INPUT DAL SERVER

                while ((line = netIn.readLine()) != null) {
                    System.out.println(line);
                }

                System.out.flush();
                System.out.print("Inserisci un numero o 'fine' per terminare: ");
                input = scanner.nextLine(); // Legge la linea completa
                s.close();

            } catch (Exception e) {
                System.err.println(e.getMessage());
                e.printStackTrace(System.err);
                System.exit(1);
            }
        }

    }
}

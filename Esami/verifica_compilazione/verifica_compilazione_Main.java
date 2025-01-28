// 18.34 18.55
import java.io.*;
import java.net.*;

public class verifica_compilazione_Main {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.err.println("Errore numero argomenti");
            System.exit(1);
        }
        try (Socket theSocket = new Socket(args[0], Integer.parseInt(args[1]))) {
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader networkIn = new BufferedReader(new InputStreamReader(theSocket.getInputStream(), "UTF-8"));
            BufferedWriter networkOut = new BufferedWriter(
                    new OutputStreamWriter(theSocket.getOutputStream(), "UTF-8"));

            // MANDO SERVER:
            for (;;) {
                System.out.println("Inserire user, 'fine' terminare");
                String nome = userIn.readLine();
                networkOut.write(nome);
                networkOut.newLine();
                networkOut.flush();
                if (nome.equals("fine"))
                    break;

                System.out.println("Inserire nome progetto, 'fine' terminare");
                String progetto = userIn.readLine();
                networkOut.write(progetto);
                networkOut.newLine();
                networkOut.flush();
                if (progetto.equals("fine"))
                    break;

                System.out.println("Inserire nome aggiornamento, 'fine' terminare");
                String aggiornamento = userIn.readLine();
                networkOut.write(aggiornamento);
                networkOut.newLine();
                networkOut.flush();
                if (aggiornamento.equals("fine"))
                    break;
                
                String theLine;
                while((theLine = networkIn.readLine())!=null){
                    if(theLine.trim().equals("==fin=="))
                       break;
                    System.out.println(theLine);
                }
            }
            networkIn.close();
            networkOut.close();
            userIn.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}

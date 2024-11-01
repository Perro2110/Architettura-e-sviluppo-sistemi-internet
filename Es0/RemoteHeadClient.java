import java.io.*;
import java.net.*;

public class RemoteHeadClient {
    public static void main(String[] args) {
        // java RemoteHeadClient nomehost porta   nomefile
        //                       args[0]  args[1] args[2]
        if (args.length != 3) {
            System.err.println("Errore! Sintassi corretta è:");
            System.err.println("\tjava RemoteHeadClient nomehost porta nomefile");
            System.exit(1);
        }

        var hostname = args[0];
        var port = args[1];
        var filename = args[2];

        try {
            var s = new Socket(hostname, Integer.parseInt(port));
            var netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));    //  ESTREMI input  SOKET   
            var netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8")); //  ESTREMI output SOKET

            //NO WRAP STDIO PERCHE MI SSREVE SOLO ARGV2[] 

            // MANDO NOME FILE AL SERVER 
            netOut.write(filename);
            netOut.newLine();
            netOut.flush(); // non necessario; performance optimization

            // RICEVO IN INPUT I FILE DAL SERVER
            String line;
            int i = 1;
            while ((line = netIn.readLine()) != null) {
                System.out.println(line);
                if (i++ == 5) break;
            }
            s.close();
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }    
}

/* 
    javac RemoteHeadClient.java
*/
/* 
SUL MIO TER: 
   ~/Scaricati java RemoteHeadClient nomehost porta   nomefile

SUL TER DEL SERVER SE NON HO GIA FILE .JAVA:
   ~/Scaricati  nc -l -p 5000   // finge di essere n server 
    pollo.txt
    aaaa
    bbbb
    eeee
    qqqq
    ssss CTRL\C
    ~/Scaricati       
 * 
 */
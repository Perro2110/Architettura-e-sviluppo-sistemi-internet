import java.io.*;
import java.net.*;

public class RemotePSClient {
    public static void main(String[] args) {
        // java RemotePSClient nomehost   porta  command_opt
        //                       args[0]  args[1] args[2]
        if (args.length < 2) {
            System.err.println("Errore! Sintassi corretta è:");
            System.err.println("\tjava RemotePSClient nomehost porta <command_opt>");
            System.exit(1);
        }

        var hostname = args[0];
        var port = args[1];
        var command_opt = "";
        if (args.length == 3)
            command_opt = args[2];
        else 
            command_opt = "null";

        try {
            var s = new Socket(hostname, Integer.parseInt(port));
            var netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));    //  ESTREMI input  SOKET   
            var netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8")); //  ESTREMI output SOKET

            //NO WRAP STDIO PERCHE MI SSREVE SOLO ARGV2[] 

            // MANDO NOME FILE AL SERVER 
            netOut.write(command_opt);
            netOut.newLine();
            netOut.flush(); // non necessario; performance optimization

            // RICEVO IN INPUT I FILE DAL SERVER
            String line;
            int i = 1;
            while ((line = netIn.readLine()) != null) {
                System.out.println(line);
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
   ~/Scaricati java RemoteHeadClient nomehost porta command_opt

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
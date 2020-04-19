using System;
using System.Text;
using System.IO;
using System.Collections.Generic;

namespace file2rle
{
    class Program
    {
        static FileStream in_file;
        static FileStream out_file;
        static byte[] out_bytes;
        static StringBuilder hex = new StringBuilder(2);
        static UInt32 out_count = 0;
        static UInt32 in_position = 0;

        const bool debug = false;

        static void Main(string[] args)
        {
            if(args.Length!=2)
            {
                Console.WriteLine("Set argument <in_file> <out_file>");
                return;
            }
            string in_filename = args[0];
            string out_filename = args[1];
            
            if (!File.Exists(in_filename))
            {
                Console.WriteLine("Infile not exist");
                return;
            }
            in_file = File.OpenRead(in_filename);
            if (in_file.Length == 0)
            {
                Console.WriteLine("Infile has zero length");
                return;
            }
            if (File.Exists(out_filename))
                File.Delete(out_filename);
            out_file = File.OpenWrite(out_filename);

            out_filename = out_filename.Substring(out_filename.LastIndexOf("\\")+1).Replace(".", "_").ToUpper();
            in_filename = in_filename.Substring(in_filename.LastIndexOf("\\")+1).Replace(".", "_").ToUpper();

            out_bytes = Encoding.UTF8.GetBytes("#ifndef __" + out_filename + "_H\r\n");
            out_file.Write(out_bytes, 0, out_bytes.Length);

            out_bytes = Encoding.UTF8.GetBytes("#define __" + out_filename + "_H\r\n");
            out_file.Write(out_bytes, 0, out_bytes.Length);

            out_bytes = Encoding.UTF8.GetBytes("\r\n#include <stdint.h>\r\n");
            out_file.Write(out_bytes, 0, out_bytes.Length);

            out_bytes = Encoding.UTF8.GetBytes("\r\nstatic const uint8_t FILES_" + in_filename + "[] = {\r\n");
            out_file.Write(out_bytes, 0, out_bytes.Length);

            int prev = in_file.ReadByte();
            int replay_count = 0;
            bool first = true;
            int prev_p = 0;
            List<byte> neg_bytes = new List<byte>();
            while (in_position < in_file.Length)
            {
                int current = in_file.ReadByte();

                if (prev == current) //повторы
                {
                    if (first)
                    {
                        replay_count++;
                        first = false;
                    }

                    if (replay_count < 0)
                    {
                        if(debug)
                            appendByteN(replay_count);
                        else
                            appendByte((byte)replay_count);
                        foreach (byte point in neg_bytes)
                            appendByte((byte)point);
                        neg_bytes.Clear();
                        replay_count = 1;
                        prev_p = -1;
                    }
                    
                    replay_count++;

                    if (replay_count == 127 || (in_position == in_file.Length - 1))
                    {
                        if (debug)
                            appendByteP(replay_count);
                        else
                            appendByte((byte)replay_count);
                        appendByte((byte)prev);
                        replay_count = 0;
                        if (prev_p == 1)  replay_count++;
                        prev_p = 0;
                    }
                }
                else //нет повторов
                {
                    if (first)
                    {
                        replay_count = 0;
                        first = false;
                    }

                    if (replay_count > 0)
                    {
                        if (prev_p == 1) replay_count++;
                        if (debug)
                            appendByteP(replay_count);
                        else
                            appendByte((byte)replay_count);
                        appendByte((byte)prev);
                        replay_count = 0;
                        prev_p = 1;
                    }
                    else
                    {
                        neg_bytes.Add((byte)prev);
                        replay_count--;
                    }

                    if (replay_count == -127 || (in_position == in_file.Length - 1))
                    {
                        if (debug)
                            appendByteN(replay_count);
                        else
                            appendByte((byte)replay_count);
                        foreach (byte point in neg_bytes)
                            appendByte((byte)point);
                        neg_bytes.Clear();
                        replay_count = 0;
                        prev_p = 0;
                    }
                }

                in_position++;
                prev = current;
            }

            out_bytes = Encoding.UTF8.GetBytes("\r\n};\r\n");
            out_file.Write(out_bytes, 0, out_bytes.Length);

            out_bytes = Encoding.UTF8.GetBytes("\r\n#endif\r\n");
            out_file.Write(out_bytes, 0, out_bytes.Length);

            in_file.Close();
            out_file.Close();
            Console.WriteLine("End.");
        }

        static void appendByteN(int data)
        {
            out_bytes = Encoding.UTF8.GetBytes("N" + data.ToString() + ", ");
            out_file.Write(out_bytes, 0, out_bytes.Length);
            out_count++;

            if ((out_count % 32) == 0)
            {
                out_bytes = Encoding.UTF8.GetBytes("\r\n");
                out_file.Write(out_bytes, 0, out_bytes.Length);
            }
        }

        static void appendByteP(int data)
        {
            out_bytes = Encoding.UTF8.GetBytes("P" + data.ToString() + ", ");
            out_file.Write(out_bytes, 0, out_bytes.Length);
            out_count++;

            if ((out_count % 32) == 0)
            {
                out_bytes = Encoding.UTF8.GetBytes("\r\n");
                out_file.Write(out_bytes, 0, out_bytes.Length);
            }
        }
        static void appendByte (byte data)
        {
            hex.Clear();
            hex.AppendFormat("{0:x2}", data);
            out_bytes = Encoding.UTF8.GetBytes("0x" + hex.ToString().ToUpper() + ", ");
            out_file.Write(out_bytes, 0, out_bytes.Length);
            out_count++;

            if ((out_count % 32) == 0)
            {
                out_bytes = Encoding.UTF8.GetBytes("\r\n/*"+ in_position +"*/ ");
                out_file.Write(out_bytes, 0, out_bytes.Length);
            }
        }
    }
}

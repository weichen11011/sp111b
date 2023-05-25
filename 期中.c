namespace b32
{
    public partial class frmMainForm : Form
    {
        private string SourceProgram; //將B32讀取到的資料保存到內存中
        private System.Collections.Hashtable LabelTable; //處理雜湊label
        private int CurrentNdx; //指向目前的位置
        private ushort AsLength; //追蹤我們的二進制程序有多大
        private bool IsEnd; //結束
        private ushort ExecutionAddress; //保存執行過的address

        private enum Registers
        {
            Unknown = 0,
            A = 4,
            B = 2,
            D = 1,
            X = 16,
            Y = 8
        }

        private void Parse(System.IO.BinaryWriter OutputFile)
        {
            CurrentNdx = 0;
            while (IsEnd == false)
                LabelScan(OutputFile, true);
            IsEnd = false;
            CurrentNdx = 0;
            AsLength = Convert.ToUInt16(this.txtOrigin.Text, 16);
            while (IsEnd == false)
                LabelScan(OutputFile, false);
        }

        private void LabelScan(System.IO.BinaryWriter OutputFile, bool IsLabelScan)
        {
            if (char.IsLetter(SourceProgram[CurrentNdx]))
            {
                // Must be a label 
                if (IsLabelScan) LabelTable.Add(GetLabelName(), AsLength);
                while (SourceProgram[CurrentNdx] != '\n')
                    CurrentNdx++;
                CurrentNdx++;
                return;
            }
            EatWhiteSpaces();
            ReadMneumonic(OutputFile, IsLabelScan);
        }

        private void ReadMneumonic(System.IO.BinaryWriter OutputFile, bool IsLabelScan)
        {
            string Mneumonic = "";
            while (!(char.IsWhiteSpace(SourceProgram[CurrentNdx])))
            {
                Mneumonic = Mneumonic + SourceProgram[CurrentNdx];
                CurrentNdx++;
            }
            if (Mneumonic.ToUpper() == "LDX") InterpretLDX(OutputFile, IsLabelScan);
            if (Mneumonic.ToUpper() == "LDA") InterpretLDA(OutputFile, IsLabelScan);
            if (Mneumonic.ToUpper() == "STA") InterpretSTA(OutputFile, IsLabelScan);
            if (Mneumonic.ToUpper() == "END")
            {
                IsEnd = true;
                DoEnd(OutputFile, IsLabelScan); EatWhiteSpaces(); ExecutionAddress =
                (ushort)LabelTable[(GetLabelName())]; return;
            }
            while (SourceProgram[CurrentNdx] != '\n')
            {
                CurrentNdx++;
            }
            CurrentNdx++;
        }

        private void InterpretLDA(System.IO.BinaryWriter OutputFile, bool IsLabelScan)
        {
            EatWhiteSpaces();
            if (SourceProgram[CurrentNdx] == '#')
            {
                CurrentNdx++;
                byte val = ReadByteValue();
                AsLength += 2;
                if (!IsLabelScan)
                {
                    OutputFile.Write((byte)0x01);
                    OutputFile.Write(val);
                }
            }
        }

        private void InterpretLDX(System.IO.BinaryWriter OutputFile, bool IsLabelScan)
        {
            EatWhiteSpaces();
            if (SourceProgram[CurrentNdx] == '#')
            {
                CurrentNdx++;
                ushort val = ReadWordValue();
                AsLength += 3;
                if (!IsLabelScan)
                {
                    OutputFile.Write((byte)0x02);
                    OutputFile.Write(val);
                }
            }
        }

        private void InterpretSTA(System.IO.BinaryWriter OutputFile, bool IsLabelScan)
        {
            EatWhiteSpaces();
            if (SourceProgram[CurrentNdx] == ',')
            {
                Registers r;
                byte opcode = 0x00;
                CurrentNdx++;
                EatWhiteSpaces();
                r = ReadRegister();
                switch (r)
                {
                    case Registers.X:
                        opcode = 0x03;
                        break;
                }
                AsLength += 1;
                if (!IsLabelScan)
                {
                    OutputFile.Write(opcode);
                }
            }
        }

        private void DoEnd(System.IO.BinaryWriter OutputFile, bool IsLabelScan)
        {
            AsLength++;
            if (!IsLabelScan)
            {
                OutputFile.Write((byte)0x04);
            }
        }

        private Registers ReadRegister()
        {
            Registers r = Registers.Unknown;
            if ((SourceProgram[CurrentNdx] == 'X') ||
            (SourceProgram[CurrentNdx] == 'x')) r = Registers.X;
            if ((SourceProgram[CurrentNdx] == 'Y') ||
            (SourceProgram[CurrentNdx] == 'y')) r = Registers.Y;
            if ((SourceProgram[CurrentNdx] == 'D') ||
            (SourceProgram[CurrentNdx] == 'd')) r = Registers.D;
            if ((SourceProgram[CurrentNdx] == 'A') ||
            (SourceProgram[CurrentNdx] == 'a')) r = Registers.A;
            if ((SourceProgram[CurrentNdx] == 'B') ||
            (SourceProgram[CurrentNdx] == 'b')) r = Registers.B;
            CurrentNdx++;
            return r;
        }

        private ushort ReadWordValue()
        {
            ushort val = 0;
            bool IsHex = false;
            string sval = "";
            if (SourceProgram[CurrentNdx] == '$')
            {
                CurrentNdx++;
                IsHex = true;
            }
            while (char.IsLetterOrDigit(SourceProgram[CurrentNdx]))
            {
                sval = sval + SourceProgram[CurrentNdx];
                CurrentNdx++;
            }
            if (IsHex)
            {
                val = Convert.ToUInt16(sval, 16);
            }
            else
            {
                val = ushort.Parse(sval);
            }
            return val;
        }

        private byte ReadByteValue()
        {
            byte val = 0;
            bool IsHex = false;
            string sval = "";
            if (SourceProgram[CurrentNdx] == '$')
            {
                CurrentNdx++;
                IsHex = true;
            }
            while (char.IsLetterOrDigit(SourceProgram[CurrentNdx]))
            {
                sval = sval + SourceProgram[CurrentNdx];
                CurrentNdx++;
            }
            if (IsHex)
            {
                val = Convert.ToByte(sval, 16);
            }
            else
            {
                val = byte.Parse(sval);
            }
            return val;
        }

        private void EatWhiteSpaces()
        {
            while (char.IsWhiteSpace(SourceProgram[CurrentNdx]))
            {
                CurrentNdx++;
            }
        }

        private string GetLabelName()
        {
            string lblname = "";
            while (char.IsLetterOrDigit(SourceProgram[CurrentNdx]))
            {
                if (SourceProgram[CurrentNdx] == ':')
                {
                    CurrentNdx++;
                    break;
                }
                lblname = lblname + SourceProgram[CurrentNdx];
                CurrentNdx++;
            }
            return lblname.ToUpper();
        }




        public frmMainForm()
        {
            InitializeComponent();
            //設初值
            LabelTable = new System.Collections.Hashtable(50);
            CurrentNdx = 0;
            AsLength = 0;
            ExecutionAddress = 0;
            IsEnd = false;
            SourceProgram = "";
            txtOrigin.Text = "1000";
        }


        private void btnsourcebrowse_Click(object sender, EventArgs e)
        {
            this.fdSourceFile.ShowDialog();
            this.txtSourceFileName.Text = fdSourceFile.FileName;
        }

        private void btnoutputbrowse_Click(object sender, EventArgs e)
        {
             this.fdDestinationFile.ShowDialog();
            this.txtOutputFileName.Text = fdDestinationFile.FileName;
        }

        private void btnassemble_Click(object sender, EventArgs e)
        {
            AsLength = Convert.ToUInt16(this.txtOrigin.Text, 16); //轉為16進制
            System.IO.BinaryWriter output;
            System.IO.TextReader input;
            System.IO.FileStream fs = new
            System.IO.FileStream(this.txtOutputFileName.Text, System.IO.FileMode.Create);
            output = new System.IO.BinaryWriter(fs);
            input = System.IO.File.OpenText(this.txtSourceFileName.Text);
            SourceProgram = input.ReadToEnd();
            input.Close();
            output.Write('B');
            output.Write('3');
            output.Write('2');
            output.Write(Convert.ToUInt16(this.txtOrigin.Text, 16));
            output.Write((ushort)0);
            Parse(output);
            output.Seek(5, System.IO.SeekOrigin.Begin);
            output.Write(ExecutionAddress);
            output.Close();
            fs.Close();
            MessageBox.Show("Done!");
        }
    }
}

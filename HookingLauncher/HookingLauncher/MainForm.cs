using System.Runtime.InteropServices;
using System.Diagnostics;
using Microsoft.Win32;
using System.IO;
using System.Text;

namespace HookingLauncher
{
    public partial class MainForm : Form
    {
        [DllImport("HookingLauncherDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public static extern bool StartHook();

        [DllImport("HookingLauncherDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public static extern bool StopHook();

        [DllImport("Kernel32")]
        private static extern long WritePrivateProfileString(string section, string key, string val, string filePath);

        [DllImport("Kernel32")]
        private static extern long GetPrivateProfileString(string section, string key, string def, StringBuilder reVal, int size, string filePath);

        private string m_FilePath = "D:\\workspace\\test\\MyDLL\\Debug\\Setting.ini";
        private bool m_bActivated = false;

        public MainForm()
        {
            InitializeComponent();

            //try
            //{
            //    string regPath = @"Software\HookingDLL";
            //    string regKey = "Setting_Dir";

            //    RegistryKey reg = Registry.LocalMachine.OpenSubKey(regPath, true);
            //    if (reg == null)
            //    {
            //        reg = Registry.LocalMachine.CreateSubKey(regPath);
            //    }

            //    reg.SetValue(regKey, m_FilePath);
            //    reg.Close();
            //}
            //catch (Exception ex)
            //{
            //    Debug.WriteLine(ex.Message);
            //    Debugger.Break();
            //}

            FormClosing += new FormClosingEventHandler(Closing);

            FileStream stream = File.Create(m_FilePath);
            stream.Close();

            if (!File.Exists(m_FilePath))
            {
                Debugger.Break();
            }

            // font string.
            WritePrivateProfileString("Watermark-String", "String", "test", m_FilePath);

            // font family.
            WritePrivateProfileString("Watermark-String", "Family", "¸¼Àº °íµñ", m_FilePath);

            // font size.
            WritePrivateProfileString("Watermark-String", "Size", "60", m_FilePath);

            // font style.
            WritePrivateProfileString("Watermark-String", "Style", "0", m_FilePath);

            // font unit.
            WritePrivateProfileString("Watermark-String", "Unit", "3", m_FilePath);

            // font brush.
            WritePrivateProfileString("Watermark-String", "Color", "536805376", m_FilePath); // 0x1FFF0000

            // image path.
            WritePrivateProfileString("Watermark-Image", "Path", "D:\\workspace\\test\\MyDLL\\MyDLL\\sample.bmp", m_FilePath);

            // image alpha value.
            WritePrivateProfileString("Watermark-Image", "Alpha", "31", m_FilePath);
        }

        ~MainForm()
        {
        }

        private void WatermarkImageSet_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.InitialDirectory = "c:\\";
                //openFileDialog.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
                openFileDialog.Filter = "webp (*.webp)|*.webp|jpeg (*.jpeg)|*.jpeg|png (*.png)|*.png|bmp (*.bmp)|*.bmp";
                openFileDialog.FilterIndex = 2;
                openFileDialog.RestoreDirectory = true;

                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    WatermarkImagePath.Text = openFileDialog.FileName;
                    WritePrivateProfileString("Watermark-Image", "Path", openFileDialog.FileName, m_FilePath);
                }
            }
        }

        private void WatermarkStringSet_Click(object sender, EventArgs e)
        {
            if (WatermarkString.Text.Length > 0)
            {
                WritePrivateProfileString("Watermark-String", "String", WatermarkString.Text, m_FilePath);
            }
        }

        private void StartHooking_Click(object sender, EventArgs e)
        {
            if (!StartHook())
            {
                MessageBox.Show("Can't start hooking!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            m_bActivated = true;
        }

        private void StopHooking_Click(object sender, EventArgs e)
        {
            if (!m_bActivated)
            {
                MessageBox.Show("Can't stop hooking!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            if (!StopHook())
            {
                Debugger.Break();
            }
            m_bActivated = false;
        }

        private void OptionBox_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void WatermarkStringOpt_Click(object sender, EventArgs e)
        {
            FontDialog fontDialog = new FontDialog();
            fontDialog.ShowColor = true;

            if (fontDialog.ShowDialog() != DialogResult.Cancel)
            {
                WritePrivateProfileString("Watermark-String", "Family", fontDialog.Font.Name.ToString(), m_FilePath);
                WritePrivateProfileString("Watermark-String", "Size", ((int)fontDialog.Font.Size).ToString(), m_FilePath);
                WritePrivateProfileString("Watermark-String", "Style", ((int)fontDialog.Font.Style).ToString(), m_FilePath);
                WritePrivateProfileString("Watermark-String", "Unit", ((int)fontDialog.Font.Unit).ToString(), m_FilePath);
                WritePrivateProfileString("Watermark-String", "Color", fontDialog.Color.ToArgb().ToString(), m_FilePath);
            }
        }

        private void Closing(object sender, FormClosingEventArgs e)
        {
            StopHook();
            m_bActivated = false;
        }
    }
}

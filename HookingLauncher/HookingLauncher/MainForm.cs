using System.Runtime.InteropServices;
using System.Diagnostics;
using Microsoft.Win32;
using System.IO;
using System.Text;

namespace HookingLauncher
{
    public partial class MainForm : Form
    {
        [DllImport("..\\DLL\\HookingLauncherDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public static extern bool StartHook();

        [DllImport("..\\DLL\\HookingLauncherDLL.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public static extern bool StopHook();

        [DllImport("Kernel32")]
        private static extern long WritePrivateProfileString(string section, string key, string val, string filePath);

        [DllImport("Kernel32")]
        private static extern long GetPrivateProfileString(string section, string key, string def, StringBuilder reVal, int size, string filePath);

        private string m_SettingFilePath = "";
        private string m_SettingPath = "";
        private bool m_bActivated = false;

        public MainForm()
        {
            InitializeComponent();

            ConfigureSetting();

            FormClosing += new FormClosingEventHandler(Closing);

            InitializeWatermarkSetting();
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
                    WritePrivateProfileString("Watermark-Image", "Path", openFileDialog.FileName, m_SettingFilePath);
                }
            }
        }

        private void WatermarkStringSet_Click(object sender, EventArgs e)
        {
            if (WatermarkString.Text.Length > 0)
            {
                WritePrivateProfileString("Watermark-String", "String", WatermarkString.Text, m_SettingFilePath);
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
                return;
            }
            if (!StopHook())
            {
                Debugger.Break();
            }
            m_bActivated = false;
        }

        private void OptionBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            // No Imlementation..
        }

        private void WatermarkStringOpt_Click(object sender, EventArgs e)
        {
            FontDialog fontDialog = new FontDialog();
            fontDialog.ShowColor = true;

            if (fontDialog.ShowDialog() != DialogResult.Cancel)
            {
                WritePrivateProfileString("Watermark-String", "Family", fontDialog.Font.Name.ToString(), m_SettingFilePath);
                WritePrivateProfileString("Watermark-String", "Size", ((int)fontDialog.Font.Size).ToString(), m_SettingFilePath);
                WritePrivateProfileString("Watermark-String", "Style", ((int)fontDialog.Font.Style).ToString(), m_SettingFilePath);
                WritePrivateProfileString("Watermark-String", "Unit", ((int)fontDialog.Font.Unit).ToString(), m_SettingFilePath);
                WritePrivateProfileString("Watermark-String", "Color", fontDialog.Color.ToArgb().ToString(), m_SettingFilePath);
            }
        }

        private void Closing(object sender, FormClosingEventArgs e)
        {
            if (!m_bActivated)
            {
                return;
            }
            StopHook();
            m_bActivated = false;
        }

        private void ConfigureSetting()
        {
            string filePath = Directory.GetCurrentDirectory();
            string settingDirectoryPath = filePath + "\\..\\Settings";
            string settingFile = settingDirectoryPath + "\\setting.ini";

            if (Directory.Exists(settingDirectoryPath) == false)
            {
                Directory.CreateDirectory(settingDirectoryPath);
            }
            if (File.Exists(settingFile) == false)
            {
                File.Create(settingFile);
            }

            m_SettingPath = settingDirectoryPath;
            m_SettingFilePath = settingFile;

            WritePrivateProfileString("FileConfigure", "Injector", filePath + "\\..\\DLL\\ForceInjection.dll", m_SettingFilePath);
            WritePrivateProfileString("FileConfigure", "CallbackDLL", filePath + "\\..\\DLL\\MyDLL.dll", m_SettingFilePath);
        }

        private void InitializeWatermarkSetting()
        {
            //if (File.Exists(m_SettingFilePath))
            //{
            //    return;
            //}

            // font string.
            WritePrivateProfileString("Watermark-String", "String", "test", m_SettingFilePath);

            // font family.
            WritePrivateProfileString("Watermark-String", "Family", "¸¼Àº °íµñ", m_SettingFilePath);

            // font size.
            WritePrivateProfileString("Watermark-String", "Size", "60", m_SettingFilePath);

            // font style.
            WritePrivateProfileString("Watermark-String", "Style", "0", m_SettingFilePath);

            // font unit.
            WritePrivateProfileString("Watermark-String", "Unit", "3", m_SettingFilePath);

            // font brush.
            WritePrivateProfileString("Watermark-String", "Color", "536805376", m_SettingFilePath); // 0x1FFF0000

            // image path.
            WritePrivateProfileString("Watermark-Image", "Path", m_SettingPath + "\\sample.bmp", m_SettingFilePath);

            // image alpha value.
            WritePrivateProfileString("Watermark-Image", "Alpha", "31", m_SettingFilePath);
        }
    }
}

namespace HookingLauncher
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            StopHooking = new Button();
            StartHooking = new Button();
            WatermarkString = new TextBox();
            WatermarkImageSet = new Button();
            WatermarkStringSet = new Button();
            OptionBox = new CheckedListBox();
            WatermarkImagePath = new TextBox();
            WatermarkStringOpt = new Button();
            SuspendLayout();
            // 
            // StopHooking
            // 
            StopHooking.AccessibleName = "StopHooking";
            StopHooking.Location = new Point(398, 245);
            StopHooking.Name = "StopHooking";
            StopHooking.Size = new Size(96, 33);
            StopHooking.TabIndex = 0;
            StopHooking.Text = "Stop";
            StopHooking.UseVisualStyleBackColor = true;
            StopHooking.Click += StopHooking_Click;
            // 
            // StartHooking
            // 
            StartHooking.AccessibleName = "StartHooking";
            StartHooking.Location = new Point(296, 245);
            StartHooking.Name = "StartHooking";
            StartHooking.Size = new Size(96, 33);
            StartHooking.TabIndex = 1;
            StartHooking.Text = "Start";
            StartHooking.UseVisualStyleBackColor = true;
            StartHooking.Click += StartHooking_Click;
            // 
            // WatermarkString
            // 
            WatermarkString.AccessibleName = "WatermarkString";
            WatermarkString.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            WatermarkString.ForeColor = SystemColors.ActiveCaptionText;
            WatermarkString.Location = new Point(21, 66);
            WatermarkString.Name = "WatermarkString";
            WatermarkString.Size = new Size(254, 23);
            WatermarkString.TabIndex = 2;
            WatermarkString.Text = "test";
            // 
            // WatermarkImageSet
            // 
            WatermarkImageSet.AccessibleName = "WatermarkImageSet";
            WatermarkImageSet.Location = new Point(281, 20);
            WatermarkImageSet.Name = "WatermarkImageSet";
            WatermarkImageSet.Size = new Size(62, 27);
            WatermarkImageSet.TabIndex = 5;
            WatermarkImageSet.Text = "Search";
            WatermarkImageSet.UseVisualStyleBackColor = true;
            WatermarkImageSet.Click += WatermarkImageSet_Click;
            // 
            // WatermarkStringSet
            // 
            WatermarkStringSet.AccessibleName = "WatermarkStringSet";
            WatermarkStringSet.Location = new Point(281, 63);
            WatermarkStringSet.Name = "WatermarkStringSet";
            WatermarkStringSet.Size = new Size(62, 27);
            WatermarkStringSet.TabIndex = 7;
            WatermarkStringSet.Text = "Submit";
            WatermarkStringSet.UseVisualStyleBackColor = true;
            WatermarkStringSet.Click += WatermarkStringSet_Click;
            // 
            // OptionBox
            // 
            OptionBox.AccessibleName = "OptionBox";
            OptionBox.FormattingEnabled = true;
            OptionBox.Items.AddRange(new object[] { "test1", "test2", "test3", "test4" });
            OptionBox.Location = new Point(21, 112);
            OptionBox.Name = "OptionBox";
            OptionBox.Size = new Size(254, 94);
            OptionBox.TabIndex = 8;
            OptionBox.SelectedIndexChanged += OptionBox_SelectedIndexChanged;
            // 
            // WatermarkImagePath
            // 
            WatermarkImagePath.AccessibleName = "WatermarkImagePath";
            WatermarkImagePath.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            WatermarkImagePath.ForeColor = SystemColors.ActiveCaptionText;
            WatermarkImagePath.Location = new Point(21, 23);
            WatermarkImagePath.Name = "WatermarkImagePath";
            WatermarkImagePath.Size = new Size(254, 23);
            WatermarkImagePath.TabIndex = 9;
            WatermarkImagePath.Text = "D:\\workspace\\test\\MyDLL\\MyDLL\\sample.bmp";
            // 
            // WatermarkStringOpt
            // 
            WatermarkStringOpt.AccessibleName = "WatermarkStringOpt";
            WatermarkStringOpt.Location = new Point(349, 63);
            WatermarkStringOpt.Name = "WatermarkStringOpt";
            WatermarkStringOpt.Size = new Size(62, 27);
            WatermarkStringOpt.TabIndex = 10;
            WatermarkStringOpt.Text = "Option";
            WatermarkStringOpt.UseVisualStyleBackColor = true;
            WatermarkStringOpt.Click += WatermarkStringOpt_Click;
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(534, 300);
            Controls.Add(WatermarkStringOpt);
            Controls.Add(WatermarkImagePath);
            Controls.Add(OptionBox);
            Controls.Add(WatermarkStringSet);
            Controls.Add(WatermarkImageSet);
            Controls.Add(WatermarkString);
            Controls.Add(StartHooking);
            Controls.Add(StopHooking);
            Name = "MainForm";
            Text = "Launcher";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Button StopHooking;
        private Button StartHooking;
        private TextBox WatermarkString;
        private Button WatermarkImageSet;
        private Button WatermarkStringSet;
        private CheckedListBox OptionBox;
        private TextBox WatermarkImagePath;
        private Button WatermarkStringOpt;
    }
}

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
            WatermarkImagePath = new TextBox();
            WatermarkStringOpt = new Button();
            ImageLabel = new Label();
            TextLabel = new Label();
            AlphaControl = new TrackBar();
            MinAlpha = new Label();
            MaxAlpha = new Label();
            AlphaLabel = new Label();
            ((System.ComponentModel.ISupportInitialize)AlphaControl).BeginInit();
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
            WatermarkString.Location = new Point(21, 116);
            WatermarkString.Name = "WatermarkString";
            WatermarkString.Size = new Size(254, 23);
            WatermarkString.TabIndex = 2;
            WatermarkString.Text = "test";
            // 
            // WatermarkImageSet
            // 
            WatermarkImageSet.AccessibleName = "WatermarkImageSet";
            WatermarkImageSet.Location = new Point(281, 57);
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
            WatermarkStringSet.Location = new Point(281, 113);
            WatermarkStringSet.Name = "WatermarkStringSet";
            WatermarkStringSet.Size = new Size(62, 27);
            WatermarkStringSet.TabIndex = 7;
            WatermarkStringSet.Text = "Submit";
            WatermarkStringSet.UseVisualStyleBackColor = true;
            WatermarkStringSet.Click += WatermarkStringSet_Click;
            // 
            // WatermarkImagePath
            // 
            WatermarkImagePath.AccessibleName = "WatermarkImagePath";
            WatermarkImagePath.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            WatermarkImagePath.ForeColor = SystemColors.ActiveCaptionText;
            WatermarkImagePath.Location = new Point(21, 60);
            WatermarkImagePath.Name = "WatermarkImagePath";
            WatermarkImagePath.Size = new Size(254, 23);
            WatermarkImagePath.TabIndex = 9;
            // 
            // WatermarkStringOpt
            // 
            WatermarkStringOpt.AccessibleName = "WatermarkStringOpt";
            WatermarkStringOpt.Location = new Point(349, 113);
            WatermarkStringOpt.Name = "WatermarkStringOpt";
            WatermarkStringOpt.Size = new Size(62, 27);
            WatermarkStringOpt.TabIndex = 10;
            WatermarkStringOpt.Text = "Option";
            WatermarkStringOpt.UseVisualStyleBackColor = true;
            WatermarkStringOpt.Click += WatermarkStringOpt_Click;
            // 
            // ImageLabel
            // 
            ImageLabel.AccessibleName = "ImageLabel";
            ImageLabel.AutoSize = true;
            ImageLabel.Location = new Point(21, 42);
            ImageLabel.Name = "ImageLabel";
            ImageLabel.Size = new Size(40, 15);
            ImageLabel.TabIndex = 11;
            ImageLabel.Text = "Image";
            // 
            // TextLabel
            // 
            TextLabel.AccessibleName = "TextLabel";
            TextLabel.AutoSize = true;
            TextLabel.Location = new Point(21, 98);
            TextLabel.Name = "TextLabel";
            TextLabel.Size = new Size(29, 15);
            TextLabel.TabIndex = 12;
            TextLabel.Text = "Text";
            // 
            // AlphaControl
            // 
            AlphaControl.AccessibleName = "AlphaControl";
            AlphaControl.Location = new Point(21, 193);
            AlphaControl.Maximum = 100;
            AlphaControl.Name = "AlphaControl";
            AlphaControl.Size = new Size(283, 45);
            AlphaControl.TabIndex = 20;
            AlphaControl.Scroll += AlphaControl_Scroll;
            // 
            // MinAlpha
            // 
            MinAlpha.AccessibleName = "MinAlpha";
            MinAlpha.AutoSize = true;
            MinAlpha.Location = new Point(21, 175);
            MinAlpha.Name = "MinAlpha";
            MinAlpha.Size = new Size(24, 15);
            MinAlpha.TabIndex = 21;
            MinAlpha.Text = "0.0";
            // 
            // MaxAlpha
            // 
            MaxAlpha.AccessibleName = "MaxAlpha";
            MaxAlpha.AutoSize = true;
            MaxAlpha.Location = new Point(281, 175);
            MaxAlpha.Name = "MaxAlpha";
            MaxAlpha.Size = new Size(24, 15);
            MaxAlpha.TabIndex = 22;
            MaxAlpha.Text = "1.0";
            // 
            // AlphaLabel
            // 
            AlphaLabel.AccessibleName = "AlphaLabel";
            AlphaLabel.AutoSize = true;
            AlphaLabel.Location = new Point(21, 155);
            AlphaLabel.Name = "AlphaLabel";
            AlphaLabel.Size = new Size(82, 15);
            AlphaLabel.TabIndex = 23;
            AlphaLabel.Text = "Alpha(투명도)";
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(534, 300);
            Controls.Add(AlphaLabel);
            Controls.Add(MaxAlpha);
            Controls.Add(MinAlpha);
            Controls.Add(AlphaControl);
            Controls.Add(TextLabel);
            Controls.Add(ImageLabel);
            Controls.Add(WatermarkStringOpt);
            Controls.Add(WatermarkImagePath);
            Controls.Add(WatermarkStringSet);
            Controls.Add(WatermarkImageSet);
            Controls.Add(WatermarkString);
            Controls.Add(StartHooking);
            Controls.Add(StopHooking);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            MaximizeBox = false;
            Name = "MainForm";
            Text = "Launcher";
            ((System.ComponentModel.ISupportInitialize)AlphaControl).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Button StopHooking;
        private Button StartHooking;
        private TextBox WatermarkString;
        private Button WatermarkImageSet;
        private Button WatermarkStringSet;
        private TextBox WatermarkImagePath;
        private Button WatermarkStringOpt;
        private Label ImageLabel;
        private Label TextLabel;
        private TrackBar AlphaControl;
        private Label MinAlpha;
        private Label MaxAlpha;
        private Label AlphaLabel;
    }
}

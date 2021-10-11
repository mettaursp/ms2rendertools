using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ApiExplorer
{
    public partial class Form1 : Form
    {
        public String SelectedType = null;
        public String SelectedMember = null;
        public String SelectedFunction = null;
        public String SelectedEnumItem = null;
        public int SelectedParameter = -1;
        public int SelectedOverload = -1;
        public int SelectedTab = 0;
        public bool RefreshLists = true;
        public bool RefreshOverloads = true;
        public bool RefreshParameters = true;
        public bool RefreshEnumItems = true;

        public void UpdateFields()
        {
            if (SelectedTab == 0 && SelectedType != null && Program.MetaData.Objects[SelectedType] == null)
                SelectedType = null;
            else if (SelectedTab == 1 && SelectedType != null && Program.MetaData.Types[SelectedType] == null)
                SelectedType = null;
            else if (SelectedTab == 2 && SelectedType != null && Program.MetaData.Enums[SelectedType] == null)
                SelectedType = null;

            if (SelectedTab != 2)
            {
                GameType type = null;
                GameMember member = null;
                List<GameFunction> function = null;
                GameFunction overload = null;
                GameFunction.Parameter parameter = null;

                if (SelectedType != null)
                {
                    if (SelectedTab == 0)
                        type = Program.MetaData.Objects[SelectedType];
                    else
                        type = Program.MetaData.Types[SelectedType];
                }

                if (type == null || SelectedMember == null || !type.Members.ContainsKey(SelectedMember))
                    SelectedMember = null;

                if (type == null || SelectedFunction == null || !type.Functions.ContainsKey(SelectedFunction))
                {
                    SelectedFunction = null;
                    SelectedParameter = -1;
                    SelectedOverload = -1;
                }
                else if (type.Functions[SelectedFunction].Count <= SelectedOverload)
                {
                    SelectedOverload = -1;
                    SelectedParameter = -1;
                }
                else if (SelectedOverload != -1)
                {
                    GameFunction func = type.Functions[SelectedFunction][SelectedOverload];

                    if (func.Parameters.Count <= SelectedParameter)
                        SelectedParameter = -1;
                }

                if (type != null)
                {
                    if (SelectedMember != null)
                        member = type.Members[SelectedMember];

                    if (SelectedFunction != null)
                        function = type.Functions[SelectedFunction];

                    if (SelectedOverload != -1)
                        overload = function[SelectedOverload];

                    if (SelectedParameter != -1)
                        parameter = overload.Parameters[SelectedParameter];
                }

                if (SelectedTab == 0)
                {
                    ObjectLabel.Text = "Name: ";

                    MemberNameLabel.Text = "Name: ";
                    MemberTypeButton.Text = "";
                    MemberDescriptionBox.Text = "";
                    MemberIsStaticBox.Checked = false;

                    IsStaticBox.Checked = false;
                    FunctionDescriptionBox.Text = "";
                    ReturnDescription.Text = "";
                    ReturnTypeBox.Text = "";
                    FunctionLabel.Text = "Name: ";

                    ParameterName.Text = "Name: ";
                    ParameterType.Text = "";
                    DefaultValue.Text = "Default Value: ";
                    ParameterDescriptionBox.Text = "";

                    if (RefreshLists)
                    {
                        MembersList.Items.Clear();
                        FunctionList.Items.Clear();
                    }

                    if (RefreshOverloads)
                        OverloadBox.Items.Clear();

                    if (RefreshParameters)
                        ParameterList.Items.Clear();

                    if (SelectedType != null)
                    {
                        ObjectLabel.Text += SelectedType;

                        if (type.Inherits != null && type.Inherits != "")
                            ObjectLabel.Text += " : " + type.Inherits;

                        ObjectDescriptionBox.Text = type.Description;

                        if (RefreshLists)
                        {
                            foreach (KeyValuePair<String, GameMember> entry in type.Members)
                                MembersList.Items.Add(entry.Key);

                            foreach (KeyValuePair<String, List<GameFunction>> entry in type.Functions)
                                FunctionList.Items.Add(entry.Key);
                        }

                        if (member != null)
                        {
                            MemberNameLabel.Text += SelectedMember;
                            MemberTypeButton.Text = member.Type;
                            MemberIsStaticBox.Checked = member.IsStatic != 0;
                            MemberDescriptionBox.Text = member.Description;

                            UpdateButton(MemberTypeButton);
                        }

                        if (RefreshOverloads && function != null)
                        {
                            for (int i = 0; i < function.Count; ++i)
                                OverloadBox.Items.Add(i);
                        }

                        if (SelectedFunction != null)
                            FunctionLabel.Text += SelectedFunction;

                        if (overload != null)
                        {
                            IsStaticBox.Checked = overload.IsStatic != 0;
                            FunctionDescriptionBox.Text = overload.Description;
                            ReturnDescription.Text = overload.ReturnValue;
                            ReturnTypeBox.Text = overload.ReturnType;

                            UpdateButton(ReturnTypeBox);

                            if (RefreshParameters)
                                for (int i = 0; i < overload.Parameters.Count; ++i)
                                    ParameterList.Items.Add(overload.Parameters[i].Name);

                            if (parameter != null)
                            {
                                ParameterName.Text += parameter.Name;
                                ParameterType.Text = parameter.Type;

                                UpdateButton(ParameterType);

                                if (parameter.DefaultValue != null)
                                    DefaultValue.Text += parameter.DefaultValue;

                                ParameterDescriptionBox.Text = parameter.Description;
                            }
                        }
                    }
                    else
                    {
                        ObjectDescriptionBox.Text = "";
                    }
                }
                else
                {
                    TypeLabel.Text = "Name: ";

                    MemberName.Text = "Name: ";
                    MemberType.Text = "";
                    MemberDescription.Text = "";
                    MemberIsStatic.Checked = false;

                    OverloadIsStatic.Checked = false;
                    TypeFunctionDescription.Text = "";
                    TypeReturnDescription.Text = "";
                    ReturnType.Text = "";
                    FunctionName.Text = "Name: ";

                    TypeParameterName.Text = "Name: ";
                    TypeParameterType.Text = "";
                    DefaultLabel.Text = "Default Value: ";
                    TypeParameterDescription.Text = "";

                    if (RefreshLists)
                    {
                        TypeMembers.Items.Clear();
                        Functions.Items.Clear();
                    }

                    if (RefreshOverloads)
                        FunctionOverloads.Items.Clear();

                    if (RefreshParameters)
                        TypeFunctionParameters.Items.Clear();

                    if (SelectedType != null)
                    {
                        TypeLabel.Text += SelectedType;

                        if (type.Inherits != null && type.Inherits != "")
                            TypeLabel.Text += " : " + type.Inherits;

                        TypeDescription.Text = type.Description;

                        if (RefreshLists)
                        {
                            foreach (KeyValuePair<String, GameMember> entry in type.Members)
                                TypeMembers.Items.Add(entry.Key);

                            foreach (KeyValuePair<String, List<GameFunction>> entry in type.Functions)
                                Functions.Items.Add(entry.Key);
                        }

                        if (member != null)
                        {
                            MemberName.Text += SelectedMember;
                            MemberType.Text = member.Type;
                            MemberIsStatic.Checked = member.IsStatic != 0;
                            MemberDescription.Text = member.Description;

                            UpdateButton(MemberType);
                        }

                        if (RefreshOverloads && function != null)
                        {
                            for (int i = 0; i < function.Count; ++i)
                                FunctionOverloads.Items.Add(i);
                        }

                        if (SelectedFunction != null)
                            FunctionName.Text += SelectedFunction;

                        if (overload != null)
                        {
                            OverloadIsStatic.Checked = overload.IsStatic != 0;
                            TypeFunctionDescription.Text = overload.Description;
                            TypeReturnDescription.Text = overload.ReturnValue;
                            ReturnType.Text = overload.ReturnType;

                            UpdateButton(ReturnType);

                            if (RefreshParameters)
                                for (int i = 0; i < overload.Parameters.Count; ++i)
                                    TypeFunctionParameters.Items.Add(overload.Parameters[i].Name);

                            if (parameter != null)
                            {
                                TypeParameterName.Text += parameter.Name;
                                TypeParameterType.Text = parameter.Type;

                                UpdateButton(TypeParameterType);

                                if (parameter.DefaultValue != null)
                                    DefaultLabel.Text += parameter.DefaultValue;

                                TypeParameterDescription.Text = parameter.Description;
                            }
                        }
                    }
                    else
                    {
                        TypeDescription.Text = "";
                    }
                }
            }
            else
            {
                List<GameEnum> enumType = null;
                GameEnum item = null;

                if (SelectedType != null)
                    enumType = Program.MetaData.Enums[SelectedType];

                if (RefreshEnumItems)
                {
                    EnumItemList.Items.Clear();

                    if (enumType != null)
                        for (int i = 0; i < enumType.Count; ++i)
                            EnumItemList.Items.Add(enumType[i].Name);
                }

                if (enumType != null)
                {
                    for (int i = 0; i < enumType.Count; ++i)
                        if (enumType[i].Name == SelectedEnumItem)
                            item = enumType[i];
                }

                EnumLabel.Text = "Name: ";
                EnumItem.Text = "Enum Item: ";
                ValueLabel.Text = "Value: ";
                EnumDescription.Text = "";

                if (enumType != null)
                    EnumLabel.Text += SelectedType;

                if (item != null)
                {
                    EnumItem.Text += item.Name;
                    ValueLabel.Text += item.Value.ToString();
                    EnumDescription.Text = "";
                }
            }

            bool refreshedOverloads = RefreshOverloads;

            RefreshLists = false;
            RefreshOverloads = false;
            RefreshParameters = false;
            RefreshEnumItems = false;

            if (refreshedOverloads)
            {
                if (SelectedTab == 0)
                {
                    if (OverloadBox.Items.Count > 0)
                    {
                        //OverloadBox.Select(Math.Max(Math.Min(OverloadBox.Items.Count, SelectedOverload), 0), 1);
                        OverloadBox.SelectedIndex = Math.Max(Math.Min(OverloadBox.Items.Count, SelectedOverload), 0);
                        //SelectedOverload = OverloadBox.SelectedIndex;
                    }
                }
                else if (SelectedTab == 1)
                {
                    if (FunctionOverloads.Items.Count > 0)
                    {
                        //FunctionOverloads.Select(Math.Max(Math.Min(FunctionOverloads.Items.Count, SelectedOverload), 0), 1);
                        FunctionOverloads.SelectedIndex = Math.Max(Math.Min(FunctionOverloads.Items.Count, SelectedOverload), 0);
                        //SelectedOverload = FunctionOverloads.SelectedIndex;
                    }
                }
            }
        }

        public Form1()
        {
            InitializeComponent();

            foreach (KeyValuePair<String, GameType> entry in Program.MetaData.Objects)
            {
                // do something with entry.Value or entry.Key

                ObjectList.Items.Add(entry.Key);
            }

            foreach (KeyValuePair<String, GameType> entry in Program.MetaData.Types)
            {
                // do something with entry.Value or entry.Key

                TypesList.Items.Add(entry.Key);
            }

            foreach (KeyValuePair<String, List<GameEnum>> entry in Program.MetaData.Enums)
            {
                // do something with entry.Value or entry.Key

                EnumList.Items.Add(entry.Key);
            }
        }

        private void ApiTabs_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (SelectedTab != ApiTabs.SelectedIndex)
            {
                if (ApiTabs.SelectedIndex == 0)
                {
                    if (ObjectList.SelectedItem != null)
                        SelectedType = ObjectList.GetItemText(ObjectList.SelectedItem);
                    else
                        SelectedType = null;
                }
                else if (ApiTabs.SelectedIndex == 1)
                {
                    if (TypesList.SelectedItem != null)
                        SelectedType = TypesList.GetItemText(TypesList.SelectedItem);
                    else
                        SelectedType = null;
                }
                else
                {
                    if (EnumList.SelectedItem != null)
                        SelectedType = EnumList.GetItemText(EnumList.SelectedItem);
                    else
                        SelectedType = null;
                }
            }

            SelectedTab = ApiTabs.SelectedIndex;
            RefreshLists = true;
            RefreshOverloads = true;
            RefreshParameters = true;
            RefreshEnumItems = true;

            UpdateFields();
        }

        private void ObjectList_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedType = ObjectList.GetItemText(ObjectList.SelectedItem);

            RefreshLists = true;

            UpdateFields();
        }

        private void TypesList_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedType = TypesList.GetItemText(TypesList.SelectedItem);

            RefreshLists = true;

            UpdateFields();
        }

        private void FunctionList_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedFunction = FunctionList.GetItemText(FunctionList.SelectedItem);

            RefreshOverloads = true;
            RefreshParameters = true;

            UpdateFields();
        }

        private void Functions_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedFunction = Functions.GetItemText(Functions.SelectedItem);
            
            RefreshOverloads = true;
            RefreshParameters = true;

            UpdateFields();
        }

        private void TypeMembers_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedMember = TypeMembers.GetItemText(TypeMembers.SelectedItem);

            UpdateFields();
        }

        private void MembersList_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedMember = MembersList.GetItemText(MembersList.SelectedItem);

            UpdateFields();
        }

        private void OverloadBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (OverloadBox.SelectedItem != null)
                SelectedOverload = OverloadBox.SelectedIndex;
            else
                SelectedOverload = -1;

            RefreshParameters = true;

            UpdateFields();
        }

        private void FunctionOverloads_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (FunctionOverloads.SelectedItem != null)
                SelectedOverload = FunctionOverloads.SelectedIndex;
            else
                SelectedOverload = -1;

            RefreshParameters = true;

            UpdateFields();
        }

        private void TypeFunctionParameters_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (TypeFunctionParameters.SelectedItem != null)
                SelectedParameter = TypeFunctionParameters.SelectedIndex;
            else
                SelectedParameter = -1;

            UpdateFields();
        }

        private void ParameterList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (ParameterList.SelectedItem != null)
                SelectedParameter = ParameterList.SelectedIndex;
            else
                SelectedParameter = -1;

            UpdateFields();
        }

        private void EnumItemList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (EnumItemList.SelectedItem != null)
                SelectedEnumItem = EnumItemList.GetItemText(EnumItemList.SelectedItem);
            else
                SelectedEnumItem = null;

            UpdateFields();
        }

        private void EnumList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (EnumList.SelectedItem != null)
                SelectedType = EnumList.GetItemText(EnumList.SelectedItem);
            else
                SelectedType = null;

            RefreshEnumItems = true;

            UpdateFields();
        }

        private void SetTypeFocus(String name)
        {
            if (name.StartsWith("Enum."))
            {
                ApiTabs.SelectTab(2);

                String enumName = name.Remove(0, 5);

                for (int i = 0; i < EnumList.Items.Count; ++i)
                    if (EnumList.Items[i].Equals(enumName))
                        EnumList.SetSelected(i, true);
            }
            else if (Program.MetaData.Objects.ContainsKey(name))
            {
                ApiTabs.SelectTab(0);

                for (int i = 0; i < ObjectList.Items.Count; ++i)
                    if (ObjectList.Items[i].Equals(name))
                        ObjectList.SetSelected(i, true);
            }
            else if (Program.MetaData.Types.ContainsKey(name))
            {
                ApiTabs.SelectTab(1);

                for (int i = 0; i < TypesList.Items.Count; ++i)
                    if (TypesList.Items[i].Equals(name))
                        TypesList.SetSelected(i, true);
            }
        }

        private void UpdateButton(Button button)
        {
            if (button.Text.StartsWith("Enum."))
                button.Enabled = true;
            else if (Program.MetaData.Objects.ContainsKey(button.Text) || Program.MetaData.Types.ContainsKey(button.Text))
                button.Enabled = true;
            else
                button.Enabled = false;
        }

        private void MemberType_Click(object sender, EventArgs e)
        {
            SetTypeFocus(MemberType.Text);
        }

        private void ReturnType_Click(object sender, EventArgs e)
        {
            SetTypeFocus(ReturnType.Text);
        }

        private void TypeParameterType_Click(object sender, EventArgs e)
        {
            SetTypeFocus(TypeParameterType.Text);
        }

        private void MemberTypeButton_Click(object sender, EventArgs e)
        {
            SetTypeFocus(MemberTypeButton.Text);
        }

        private void ReturnTypeBox_Click(object sender, EventArgs e)
        {
            SetTypeFocus(ReturnTypeBox.Text);
        }

        private void ParameterType_Click(object sender, EventArgs e)
        {
            SetTypeFocus(ParameterType.Text);
        }
    }
}

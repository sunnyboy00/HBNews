//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

// Generated from: gs_c_msg_body.proto
namespace gs_c_msg_body
{
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"struct_map_point_info")]
  public partial class struct_map_point_info : global::ProtoBuf.IExtensible
  {
    public struct_map_point_info() {}
    
    private int _map_point_id;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"map_point_id", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int map_point_id
    {
      get { return _map_point_id; }
      set { _map_point_id = value; }
    }
    private int _not_done_branch_task;
    [global::ProtoBuf.ProtoMember(2, IsRequired = true, Name=@"not_done_branch_task", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int not_done_branch_task
    {
      get { return _not_done_branch_task; }
      set { _not_done_branch_task = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"struct_mappoint")]
  public partial class struct_mappoint : global::ProtoBuf.IExtensible
  {
    public struct_mappoint() {}
    
    private string _id;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"id", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string id
    {
      get { return _id; }
      set { _id = value; }
    }
    private string _name_ch;
    [global::ProtoBuf.ProtoMember(2, IsRequired = true, Name=@"name_ch", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string name_ch
    {
      get { return _name_ch; }
      set { _name_ch = value; }
    }
    private string _name_en = "";
    [global::ProtoBuf.ProtoMember(3, IsRequired = false, Name=@"name_en", DataFormat = global::ProtoBuf.DataFormat.Default)]
    [global::System.ComponentModel.DefaultValue("")]
    public string name_en
    {
      get { return _name_en; }
      set { _name_en = value; }
    }
    private int _chapterType = default(int);
    [global::ProtoBuf.ProtoMember(4, IsRequired = false, Name=@"chapterType", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    [global::System.ComponentModel.DefaultValue(default(int))]
    public int chapterType
    {
      get { return _chapterType; }
      set { _chapterType = value; }
    }
    private int _pointType = default(int);
    [global::ProtoBuf.ProtoMember(5, IsRequired = false, Name=@"pointType", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    [global::System.ComponentModel.DefaultValue(default(int))]
    public int pointType
    {
      get { return _pointType; }
      set { _pointType = value; }
    }
    private string _iconid = "";
    [global::ProtoBuf.ProtoMember(6, IsRequired = false, Name=@"iconid", DataFormat = global::ProtoBuf.DataFormat.Default)]
    [global::System.ComponentModel.DefaultValue("")]
    public string iconid
    {
      get { return _iconid; }
      set { _iconid = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"struct_task")]
  public partial class struct_task : global::ProtoBuf.IExtensible
  {
    public struct_task() {}
    
    private string _id;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"id", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string id
    {
      get { return _id; }
      set { _id = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"struct_sub_task")]
  public partial class struct_sub_task : global::ProtoBuf.IExtensible
  {
    public struct_sub_task() {}
    
    private string _id;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"id", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string id
    {
      get { return _id; }
      set { _id = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_config_req")]
  public partial class gs_c_config_req : global::ProtoBuf.IExtensible
  {
    public gs_c_config_req() {}
    
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_config_res")]
  public partial class gs_c_config_res : global::ProtoBuf.IExtensible
  {
    public gs_c_config_res() {}
    
    private readonly global::System.Collections.Generic.List<struct_mappoint> _mappoints = new global::System.Collections.Generic.List<struct_mappoint>();
    [global::ProtoBuf.ProtoMember(1, Name=@"mappoints", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public global::System.Collections.Generic.List<struct_mappoint> mappoints
    {
      get { return _mappoints; }
    }
  
    private readonly global::System.Collections.Generic.List<struct_task> _tasks = new global::System.Collections.Generic.List<struct_task>();
    [global::ProtoBuf.ProtoMember(2, Name=@"tasks", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public global::System.Collections.Generic.List<struct_task> tasks
    {
      get { return _tasks; }
    }
  
    private readonly global::System.Collections.Generic.List<struct_sub_task> _sub_tasks = new global::System.Collections.Generic.List<struct_sub_task>();
    [global::ProtoBuf.ProtoMember(3, Name=@"sub_tasks", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public global::System.Collections.Generic.List<struct_sub_task> sub_tasks
    {
      get { return _sub_tasks; }
    }
  
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_register_req")]
  public partial class gs_c_register_req : global::ProtoBuf.IExtensible
  {
    public gs_c_register_req() {}
    
    private int _plantform_id;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"plantform_id", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int plantform_id
    {
      get { return _plantform_id; }
      set { _plantform_id = value; }
    }
    private int _plantform_sub_id;
    [global::ProtoBuf.ProtoMember(2, IsRequired = true, Name=@"plantform_sub_id", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int plantform_sub_id
    {
      get { return _plantform_sub_id; }
      set { _plantform_sub_id = value; }
    }
    private string _account;
    [global::ProtoBuf.ProtoMember(3, IsRequired = true, Name=@"account", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string account
    {
      get { return _account; }
      set { _account = value; }
    }
    private string _password;
    [global::ProtoBuf.ProtoMember(4, IsRequired = true, Name=@"password", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string password
    {
      get { return _password; }
      set { _password = value; }
    }
    private string _name;
    [global::ProtoBuf.ProtoMember(5, IsRequired = true, Name=@"name", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string name
    {
      get { return _name; }
      set { _name = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_register_res")]
  public partial class gs_c_register_res : global::ProtoBuf.IExtensible
  {
    public gs_c_register_res() {}
    
    private int _error_code;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"error_code", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int error_code
    {
      get { return _error_code; }
      set { _error_code = value; }
    }
    private int _user_id;
    [global::ProtoBuf.ProtoMember(2, IsRequired = true, Name=@"user_id", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int user_id
    {
      get { return _user_id; }
      set { _user_id = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_login_req")]
  public partial class gs_c_login_req : global::ProtoBuf.IExtensible
  {
    public gs_c_login_req() {}
    
    private int _plantform_id;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"plantform_id", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int plantform_id
    {
      get { return _plantform_id; }
      set { _plantform_id = value; }
    }
    private int _plantform_sub_id;
    [global::ProtoBuf.ProtoMember(2, IsRequired = true, Name=@"plantform_sub_id", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int plantform_sub_id
    {
      get { return _plantform_sub_id; }
      set { _plantform_sub_id = value; }
    }
    private string _account;
    [global::ProtoBuf.ProtoMember(3, IsRequired = true, Name=@"account", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string account
    {
      get { return _account; }
      set { _account = value; }
    }
    private string _password;
    [global::ProtoBuf.ProtoMember(4, IsRequired = true, Name=@"password", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string password
    {
      get { return _password; }
      set { _password = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_login_res")]
  public partial class gs_c_login_res : global::ProtoBuf.IExtensible
  {
    public gs_c_login_res() {}
    
    private int _error_code = default(int);
    [global::ProtoBuf.ProtoMember(1, IsRequired = false, Name=@"error_code", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    [global::System.ComponentModel.DefaultValue(default(int))]
    public int error_code
    {
      get { return _error_code; }
      set { _error_code = value; }
    }
    private int _user_id = default(int);
    [global::ProtoBuf.ProtoMember(2, IsRequired = false, Name=@"user_id", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    [global::System.ComponentModel.DefaultValue(default(int))]
    public int user_id
    {
      get { return _user_id; }
      set { _user_id = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_chapter_map_info_req")]
  public partial class gs_c_chapter_map_info_req : global::ProtoBuf.IExtensible
  {
    public gs_c_chapter_map_info_req() {}
    
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_chapter_map_info_res")]
  public partial class gs_c_chapter_map_info_res : global::ProtoBuf.IExtensible
  {
    public gs_c_chapter_map_info_res() {}
    
    private int _error_code;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"error_code", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int error_code
    {
      get { return _error_code; }
      set { _error_code = value; }
    }
    private int _cur_main_task_map_point;
    [global::ProtoBuf.ProtoMember(2, IsRequired = true, Name=@"cur_main_task_map_point", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int cur_main_task_map_point
    {
      get { return _cur_main_task_map_point; }
      set { _cur_main_task_map_point = value; }
    }
    private readonly global::System.Collections.Generic.List<struct_map_point_info> _map_point_info = new global::System.Collections.Generic.List<struct_map_point_info>();
    [global::ProtoBuf.ProtoMember(3, Name=@"map_point_info", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public global::System.Collections.Generic.List<struct_map_point_info> map_point_info
    {
      get { return _map_point_info; }
    }
  
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_error_report")]
  public partial class gs_c_error_report : global::ProtoBuf.IExtensible
  {
    public gs_c_error_report() {}
    
    private int _error_code;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"error_code", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int error_code
    {
      get { return _error_code; }
      set { _error_code = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_user_info_req")]
  public partial class gs_c_user_info_req : global::ProtoBuf.IExtensible
  {
    public gs_c_user_info_req() {}
    
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
  [global::System.Serializable, global::ProtoBuf.ProtoContract(Name=@"gs_c_user_info_res")]
  public partial class gs_c_user_info_res : global::ProtoBuf.IExtensible
  {
    public gs_c_user_info_res() {}
    
    private int _error_code;
    [global::ProtoBuf.ProtoMember(1, IsRequired = true, Name=@"error_code", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int error_code
    {
      get { return _error_code; }
      set { _error_code = value; }
    }
    private string _name;
    [global::ProtoBuf.ProtoMember(2, IsRequired = true, Name=@"name", DataFormat = global::ProtoBuf.DataFormat.Default)]
    public string name
    {
      get { return _name; }
      set { _name = value; }
    }
    private int _level;
    [global::ProtoBuf.ProtoMember(3, IsRequired = true, Name=@"level", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int level
    {
      get { return _level; }
      set { _level = value; }
    }
    private int _gold;
    [global::ProtoBuf.ProtoMember(4, IsRequired = true, Name=@"gold", DataFormat = global::ProtoBuf.DataFormat.TwosComplement)]
    public int gold
    {
      get { return _gold; }
      set { _gold = value; }
    }
    private global::ProtoBuf.IExtension extensionObject;
    global::ProtoBuf.IExtension global::ProtoBuf.IExtensible.GetExtensionObject(bool createIfMissing)
      { return global::ProtoBuf.Extensible.GetExtensionObject(ref extensionObject, createIfMissing); }
  }
  
    [global::ProtoBuf.ProtoContract(Name=@"e_c_gs_msg")]
    public enum e_c_gs_msg
    {
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_c_gs_msg_min", Value=0)]
      e_c_gs_msg_min = 0,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_c_gs_msg_register_req", Value=1)]
      e_c_gs_msg_register_req = 1,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_c_gs_msg_login_req", Value=2)]
      e_c_gs_msg_login_req = 2,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_c_gs_msg_config_req", Value=3)]
      e_c_gs_msg_config_req = 3,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_c_gs_msg_chapter_map_info_req", Value=4)]
      e_c_gs_msg_chapter_map_info_req = 4,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_c_gs_msg_user_info_req", Value=5)]
      e_c_gs_msg_user_info_req = 5,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_c_gs_msg_max", Value=6)]
      e_c_gs_msg_max = 6
    }
  
    [global::ProtoBuf.ProtoContract(Name=@"e_gs_c_msg")]
    public enum e_gs_c_msg
    {
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_msg_min", Value=0)]
      e_gs_c_msg_min = 0,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_msg_register_res", Value=1)]
      e_gs_c_msg_register_res = 1,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_msg_login_res", Value=2)]
      e_gs_c_msg_login_res = 2,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_msg_conifg_res", Value=3)]
      e_gs_c_msg_conifg_res = 3,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_msg_chapter_map_info_res", Value=4)]
      e_gs_c_msg_chapter_map_info_res = 4,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_error_report", Value=5)]
      e_gs_c_error_report = 5,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_msg_user_info_res", Value=6)]
      e_gs_c_msg_user_info_res = 6,
            
      [global::ProtoBuf.ProtoEnum(Name=@"e_gs_c_msg_max", Value=7)]
      e_gs_c_msg_max = 7
    }
  
    [global::ProtoBuf.ProtoContract(Name=@"EGSCErrorCode")]
    public enum EGSCErrorCode
    {
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_MIN", Value=0)]
      EErrorCode_MIN = 0,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_Success", Value=1)]
      EErrorCode_Success = 1,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_UnKown_Error", Value=2)]
      EErrorCode_UnKown_Error = 2,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_Registe_Already", Value=3)]
      EErrorCode_Registe_Already = 3,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_Account_Not_Exist_Or_Wrong_Password", Value=4)]
      EErrorCode_Account_Not_Exist_Or_Wrong_Password = 4,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_User_Data_Error", Value=5)]
      EErrorCode_User_Data_Error = 5,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_Should_Login_Before_Other", Value=6)]
      EErrorCode_Should_Login_Before_Other = 6,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_Has_Logined", Value=7)]
      EErrorCode_Has_Logined = 7,
            
      [global::ProtoBuf.ProtoEnum(Name=@"EErrorCode_TASKCONFIG_ERROR", Value=8)]
      EErrorCode_TASKCONFIG_ERROR = 8
    }
  
}
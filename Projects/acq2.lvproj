<?xml version='1.0'?>
<Project Type="Project" LVVersion="8008005">
   <Property Name="CCSymbols" Type="Str"></Property>
   <Property Name="NI.Project.Description" Type="Str"></Property>
   <Item Name="My Computer" Type="My Computer">
      <Property Name="CCSymbols" Type="Str">OS,Win;CPU,x86;</Property>
      <Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
      <Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
      <Property Name="server.tcp.enabled" Type="Bool">false</Property>
      <Property Name="server.tcp.port" Type="Int">0</Property>
      <Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
      <Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
      <Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
      <Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
      <Property Name="specify.custom.address" Type="Bool">false</Property>
      <Item Name="SharedVars.lvlib" Type="Library" URL="SharedVars.lvlib">
         <Item Name="ExitFlag" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="Ch1Spectrum" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="ChamberTemp" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="AmbientTemp" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="SUB_Ch1Display1.vi" Type="VI" URL="../171005/SUB_Ch1Display1.vi"/>
         <Item Name="Ch1DataGraph1" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="Ch1StimGraph" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="Ch1Color" Type="Variable"/>
         <Item Name="Ch1GraphProps" Type="Variable"/>
         <Item Name="Ch1Info" Type="Variable"/>
         <Item Name="SUB_InfoDisplay.vi" Type="VI" URL="../171005/SUB_InfoDisplay.vi"/>
         <Item Name="Ch1Update" Type="Variable"/>
         <Item Name="Ch1Draw" Type="Variable"/>
         <Item Name="Ch1OscMult" Type="Variable"/>
         <Item Name="SUB_Ch1Display2.vi" Type="VI" URL="../171005/SUB_Ch1Display2.vi"/>
         <Item Name="Ch1DataGraph2" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="Ch2Draw" Type="Variable"/>
         <Item Name="Ch1Title" Type="Variable"/>
         <Item Name="Ch2Title" Type="Variable"/>
         <Item Name="PressureArray" Type="Variable"/>
         <Item Name="CurPressure" Type="Variable"/>
         <Item Name="SUB_Ch2Display1.vi" Type="VI" URL="../171005/SUB_Ch2Display1.vi"/>
         <Item Name="SUB_Ch2Display2.vi" Type="VI" URL="../171005/SUB_Ch2Display2.vi"/>
         <Item Name="Ch2DataGraph1" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="Ch2DataGraph2" Type="Variable">
            <Property Name="NI.Lib.ShowInTree" Type="Bool">true</Property>
         </Item>
         <Item Name="sg100.vi" Type="VI" URL="../171005/sg100.vi"/>
         <Item Name="sg95_2.vi" Type="VI" URL="../171005/sg95_2.vi"/>
      </Item>
      <Item Name="GraphProp.ctl" Type="VI" URL="../171005/GraphProp.ctl"/>
      <Item Name="RemoteGraph.ctl" Type="VI" URL="../171005/RemoteGraph.ctl"/>
      <Item Name="Dependencies" Type="Dependencies"/>
      <Item Name="Build Specifications" Type="Build"/>
   </Item>
</Project>

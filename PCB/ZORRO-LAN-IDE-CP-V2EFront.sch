<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="6.5.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="2" name="Route2" color="1" fill="3" visible="no" active="no"/>
<layer number="3" name="Route3" color="4" fill="3" visible="no" active="no"/>
<layer number="4" name="Route4" color="1" fill="4" visible="no" active="no"/>
<layer number="5" name="Route5" color="4" fill="4" visible="no" active="no"/>
<layer number="6" name="Route6" color="1" fill="8" visible="no" active="no"/>
<layer number="7" name="Route7" color="4" fill="8" visible="no" active="no"/>
<layer number="8" name="Route8" color="1" fill="2" visible="no" active="no"/>
<layer number="9" name="Route9" color="4" fill="2" visible="no" active="no"/>
<layer number="10" name="Route10" color="1" fill="7" visible="no" active="no"/>
<layer number="11" name="Route11" color="4" fill="7" visible="no" active="no"/>
<layer number="12" name="Route12" color="1" fill="5" visible="no" active="no"/>
<layer number="13" name="Route13" color="4" fill="5" visible="no" active="no"/>
<layer number="14" name="Route14" color="1" fill="6" visible="no" active="no"/>
<layer number="15" name="Route15" color="4" fill="6" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="9" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="con-stewart">
<description>&lt;b&gt;Bel Stewart Conector&lt;/b&gt;&lt;p&gt;
www.stewartconnector.com&lt;br&gt;
11118 susquehanna Trail, South&lt;br&gt;
Glen Rock, Pa 17327-9199&lt;br&gt;
717.234.-7512&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="SI-50170">
<description>&lt;b&gt;Ethernet Connector (MagJack)&lt;/b&gt;&lt;p&gt;
Source: si-50170.pdf</description>
<wire x1="-7.975" y1="11.475" x2="7.975" y2="11.475" width="0.2032" layer="21"/>
<wire x1="7.975" y1="11.475" x2="7.975" y2="1.35" width="0.2032" layer="21"/>
<wire x1="7.975" y1="1.35" x2="7.975" y2="-1.25" width="0.2032" layer="51"/>
<wire x1="7.975" y1="-1.25" x2="7.975" y2="-13.75" width="0.2032" layer="21"/>
<wire x1="7.975" y1="-13.75" x2="-7.975" y2="-13.75" width="0.2032" layer="21"/>
<wire x1="-7.975" y1="-13.75" x2="-7.975" y2="-1.25" width="0.2032" layer="21"/>
<wire x1="-7.975" y1="-1.25" x2="-7.975" y2="1.25" width="0.2032" layer="51"/>
<wire x1="-7.975" y1="1.25" x2="-7.975" y2="11.475" width="0.2032" layer="21"/>
<wire x1="-7.975" y1="-13.75" x2="-8.2" y2="-13.225" width="0.2032" layer="21"/>
<wire x1="-8.2" y1="-13.225" x2="-8.65" y2="-8.2" width="0.2032" layer="21" curve="-36.162634"/>
<wire x1="-8.65" y1="-8.2" x2="-8.375" y2="-6.675" width="0.2032" layer="21" curve="5.484358"/>
<wire x1="-8.375" y1="-6.675" x2="-8.5" y2="-6.325" width="0.2032" layer="21" curve="54.265296"/>
<wire x1="7.975" y1="-13.75" x2="8.2" y2="-13.225" width="0.2032" layer="21"/>
<wire x1="8.2" y1="-13.225" x2="8.65" y2="-8.2" width="0.2032" layer="21" curve="36.162634"/>
<wire x1="8.65" y1="-8.2" x2="8.375" y2="-6.675" width="0.2032" layer="21" curve="-5.484358"/>
<wire x1="8.375" y1="-6.675" x2="8.5" y2="-6.325" width="0.2032" layer="21" curve="-54.265296"/>
<pad name="S1" x="-7.747" y="0" drill="1.6" diameter="2.2"/>
<pad name="S2" x="7.747" y="0" drill="1.6" diameter="2.2"/>
<pad name="1" x="4.445" y="5.842" drill="0.9" diameter="1.5"/>
<pad name="2" x="3.175" y="3.302" drill="0.9" diameter="1.5"/>
<pad name="3" x="1.905" y="5.842" drill="0.9" diameter="1.5"/>
<pad name="4" x="0.635" y="3.302" drill="0.9" diameter="1.5"/>
<pad name="5" x="-0.635" y="5.842" drill="0.9" diameter="1.5"/>
<pad name="6" x="-1.905" y="3.302" drill="0.9" diameter="1.5"/>
<pad name="7" x="-3.175" y="5.842" drill="0.9" diameter="1.5"/>
<pad name="8" x="-4.445" y="3.302" drill="0.9" diameter="1.5"/>
<pad name="A1" x="-5.461" y="10.668" drill="1.016" diameter="1.5"/>
<pad name="C1" x="-2.921" y="10.668" drill="1.016" diameter="1.5"/>
<pad name="A2" x="2.921" y="10.668" drill="1.016" diameter="1.5"/>
<pad name="C2" x="5.461" y="10.668" drill="1.016" diameter="1.5"/>
<text x="-3.81" y="-12.065" size="1.27" layer="25">&gt;NAME</text>
<text x="-3.81" y="-9.525" size="1.27" layer="27">&gt;VALUE</text>
<hole x="-5.715" y="-3.048" drill="3.2512"/>
<hole x="5.715" y="-3.048" drill="3.2512"/>
</package>
</packages>
<symbols>
<symbol name="SSI-50170">
<wire x1="-7.62" y1="17.78" x2="-7.62" y2="-5.08" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-7.62" y2="-7.62" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-7.62" x2="-7.62" y2="-15.24" width="0.254" layer="94"/>
<wire x1="-7.62" y1="-15.24" x2="10.16" y2="-15.24" width="0.254" layer="94"/>
<wire x1="10.16" y1="-15.24" x2="10.16" y2="17.78" width="0.254" layer="94"/>
<wire x1="10.16" y1="17.78" x2="-7.62" y2="17.78" width="0.254" layer="94"/>
<wire x1="7.62" y1="-3.81" x2="7.62" y2="16.51" width="0.254" layer="94"/>
<wire x1="7.62" y1="16.51" x2="2.54" y2="16.51" width="0.254" layer="94"/>
<wire x1="2.54" y1="16.51" x2="2.54" y2="10.16" width="0.254" layer="94"/>
<wire x1="2.54" y1="10.16" x2="1.27" y2="10.16" width="0.254" layer="94"/>
<wire x1="1.27" y1="10.16" x2="1.27" y2="7.62" width="0.254" layer="94"/>
<wire x1="1.27" y1="7.62" x2="0" y2="7.62" width="0.254" layer="94"/>
<wire x1="0" y1="7.62" x2="0" y2="5.08" width="0.254" layer="94"/>
<wire x1="0" y1="5.08" x2="1.27" y2="5.08" width="0.254" layer="94"/>
<wire x1="1.27" y1="5.08" x2="1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="1.27" y1="2.54" x2="2.54" y2="2.54" width="0.254" layer="94"/>
<wire x1="2.54" y1="2.54" x2="2.54" y2="-3.81" width="0.254" layer="94"/>
<wire x1="2.54" y1="-3.81" x2="7.62" y2="-3.81" width="0.254" layer="94"/>
<wire x1="-6.096" y1="-5.461" x2="-5.08" y2="-7.112" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.112" x2="-4.064" y2="-5.461" width="0.254" layer="94"/>
<wire x1="-6.223" y1="-7.112" x2="-5.08" y2="-7.112" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-7.112" x2="-3.81" y2="-7.112" width="0.254" layer="94"/>
<wire x1="-6.096" y1="-5.461" x2="-4.064" y2="-5.461" width="0.254" layer="94"/>
<wire x1="-3.175" y1="-5.334" x2="-2.159" y2="-6.35" width="0.1524" layer="94"/>
<wire x1="-3.302" y1="-6.477" x2="-2.286" y2="-7.493" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-5.08" x2="-5.08" y2="-5.334" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-7.239" x2="-5.08" y2="-7.62" width="0.1524" layer="94"/>
<wire x1="-7.62" y1="-5.08" x2="-5.08" y2="-5.08" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-7.62" x2="-7.62" y2="-7.62" width="0.1524" layer="94"/>
<wire x1="-6.096" y1="-10.541" x2="-5.08" y2="-12.192" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-12.192" x2="-4.064" y2="-10.541" width="0.254" layer="94"/>
<wire x1="-6.223" y1="-12.192" x2="-5.08" y2="-12.192" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-12.192" x2="-3.81" y2="-12.192" width="0.254" layer="94"/>
<wire x1="-6.096" y1="-10.541" x2="-4.064" y2="-10.541" width="0.254" layer="94"/>
<wire x1="-3.175" y1="-10.414" x2="-2.159" y2="-11.43" width="0.1524" layer="94"/>
<wire x1="-3.302" y1="-11.557" x2="-2.286" y2="-12.573" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-10.16" x2="-5.08" y2="-10.414" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-12.319" x2="-5.08" y2="-12.7" width="0.1524" layer="94"/>
<wire x1="-7.62" y1="-10.16" x2="-5.08" y2="-10.16" width="0.1524" layer="94"/>
<wire x1="-5.08" y1="-12.7" x2="-7.62" y2="-12.7" width="0.1524" layer="94"/>
<text x="-7.62" y="19.05" size="1.778" layer="95" font="vector">&gt;NAME</text>
<text x="-7.62" y="-17.78" size="1.778" layer="96" font="vector">&gt;VALUE</text>
<pin name="TD+" x="-10.16" y="15.24" length="short" direction="pas"/>
<pin name="TCT" x="-10.16" y="12.7" length="short" direction="pas"/>
<pin name="TD-" x="-10.16" y="10.16" length="short" direction="pas"/>
<pin name="RD+" x="-10.16" y="7.62" length="short" direction="pas"/>
<pin name="RCT" x="-10.16" y="5.08" length="short" direction="pas"/>
<pin name="RD-" x="-10.16" y="2.54" length="short" direction="pas"/>
<pin name="NC" x="-10.16" y="0" length="short" direction="pas"/>
<pin name="C_GND" x="-10.16" y="-2.54" length="short" direction="pas"/>
<pin name="C" x="-10.16" y="-7.62" visible="pad" length="short" direction="pas"/>
<pin name="A" x="-10.16" y="-5.08" visible="pad" length="short" direction="pas"/>
<pin name="C1" x="-10.16" y="-12.7" visible="pad" length="short" direction="pas"/>
<pin name="A1" x="-10.16" y="-10.16" visible="pad" length="short" direction="pas"/>
<pin name="S1" x="5.08" y="-17.78" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="S2" x="7.62" y="-17.78" visible="pad" length="short" direction="pas" rot="R90"/>
<polygon width="0.1524" layer="94">
<vertex x="-2.159" y="-6.35"/>
<vertex x="-2.54" y="-5.461"/>
<vertex x="-3.048" y="-5.969"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="-2.286" y="-7.493"/>
<vertex x="-2.667" y="-6.604"/>
<vertex x="-3.175" y="-7.112"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="-2.159" y="-11.43"/>
<vertex x="-2.54" y="-10.541"/>
<vertex x="-3.048" y="-11.049"/>
</polygon>
<polygon width="0.1524" layer="94">
<vertex x="-2.286" y="-12.573"/>
<vertex x="-2.667" y="-11.684"/>
<vertex x="-3.175" y="-12.192"/>
</polygon>
</symbol>
</symbols>
<devicesets>
<deviceset name="SI-50170" prefix="X">
<description>&lt;b&gt;Ethernet Connector MagJack (R)&lt;/b&gt;&lt;p&gt;
Source: www.stewartconnector.com .. si-50170.pdf</description>
<gates>
<gate name="G$1" symbol="SSI-50170" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SI-50170">
<connects>
<connect gate="G$1" pin="A" pad="A1"/>
<connect gate="G$1" pin="A1" pad="A2"/>
<connect gate="G$1" pin="C" pad="C1"/>
<connect gate="G$1" pin="C1" pad="C2"/>
<connect gate="G$1" pin="C_GND" pad="8"/>
<connect gate="G$1" pin="NC" pad="7"/>
<connect gate="G$1" pin="RCT" pad="5"/>
<connect gate="G$1" pin="RD+" pad="4"/>
<connect gate="G$1" pin="RD-" pad="6"/>
<connect gate="G$1" pin="S1" pad="S1"/>
<connect gate="G$1" pin="S2" pad="S2"/>
<connect gate="G$1" pin="TCT" pad="2"/>
<connect gate="G$1" pin="TD+" pad="1"/>
<connect gate="G$1" pin="TD-" pad="3"/>
</connects>
<technologies>
<technology name="">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="unknown" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="con-ml">
<description>&lt;b&gt;Harting  Connectors&lt;/b&gt;&lt;p&gt;
Low profile connectors, straight&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="ML14">
<description>&lt;b&gt;HARTING&lt;/b&gt;</description>
<wire x1="-11.43" y1="3.175" x2="11.43" y2="3.175" width="0.127" layer="21"/>
<wire x1="11.43" y1="-3.175" x2="11.43" y2="3.175" width="0.127" layer="21"/>
<wire x1="-11.43" y1="3.175" x2="-11.43" y2="-3.175" width="0.127" layer="21"/>
<wire x1="-12.7" y1="4.445" x2="-11.43" y2="4.445" width="0.127" layer="21"/>
<wire x1="12.7" y1="-4.445" x2="8.001" y2="-4.445" width="0.127" layer="21"/>
<wire x1="12.7" y1="-4.445" x2="12.7" y2="4.445" width="0.127" layer="21"/>
<wire x1="-12.7" y1="4.445" x2="-12.7" y2="-4.445" width="0.127" layer="21"/>
<wire x1="11.43" y1="-3.175" x2="7.112" y2="-3.175" width="0.127" layer="21"/>
<wire x1="2.032" y1="-2.413" x2="-2.032" y2="-2.413" width="0.127" layer="21"/>
<wire x1="-2.032" y1="-3.175" x2="-2.032" y2="-2.413" width="0.127" layer="21"/>
<wire x1="-2.032" y1="-3.175" x2="-11.43" y2="-3.175" width="0.127" layer="21"/>
<wire x1="-2.032" y1="-3.175" x2="-2.032" y2="-3.429" width="0.127" layer="21"/>
<wire x1="2.032" y1="-2.413" x2="2.032" y2="-3.175" width="0.127" layer="21"/>
<wire x1="2.032" y1="-3.175" x2="2.032" y2="-3.429" width="0.127" layer="21"/>
<wire x1="11.43" y1="4.445" x2="11.43" y2="4.699" width="0.127" layer="21"/>
<wire x1="11.43" y1="4.699" x2="10.16" y2="4.699" width="0.127" layer="21"/>
<wire x1="10.16" y1="4.445" x2="10.16" y2="4.699" width="0.127" layer="21"/>
<wire x1="11.43" y1="4.445" x2="12.7" y2="4.445" width="0.127" layer="21"/>
<wire x1="0.635" y1="4.699" x2="-0.635" y2="4.699" width="0.127" layer="21"/>
<wire x1="0.635" y1="4.699" x2="0.635" y2="4.445" width="0.127" layer="21"/>
<wire x1="0.635" y1="4.445" x2="10.16" y2="4.445" width="0.127" layer="21"/>
<wire x1="-0.635" y1="4.699" x2="-0.635" y2="4.445" width="0.127" layer="21"/>
<wire x1="-10.16" y1="4.699" x2="-11.43" y2="4.699" width="0.127" layer="21"/>
<wire x1="-11.43" y1="4.699" x2="-11.43" y2="4.445" width="0.127" layer="21"/>
<wire x1="-10.16" y1="4.699" x2="-10.16" y2="4.445" width="0.127" layer="21"/>
<wire x1="-10.16" y1="4.445" x2="-0.635" y2="4.445" width="0.127" layer="21"/>
<wire x1="4.699" y1="-4.445" x2="2.032" y2="-4.445" width="0.127" layer="21"/>
<wire x1="2.032" y1="-4.445" x2="-2.032" y2="-4.445" width="0.127" layer="21"/>
<wire x1="5.588" y1="-3.175" x2="5.588" y2="-3.429" width="0.127" layer="21"/>
<wire x1="5.588" y1="-3.175" x2="2.032" y2="-3.175" width="0.127" layer="21"/>
<wire x1="7.112" y1="-3.175" x2="7.112" y2="-3.429" width="0.127" layer="21"/>
<wire x1="7.112" y1="-3.175" x2="5.588" y2="-3.175" width="0.127" layer="21"/>
<wire x1="4.699" y1="-4.445" x2="5.08" y2="-3.937" width="0.127" layer="21"/>
<wire x1="7.62" y1="-3.937" x2="8.001" y2="-4.445" width="0.127" layer="21"/>
<wire x1="7.62" y1="-3.937" x2="7.112" y2="-3.937" width="0.127" layer="21"/>
<wire x1="5.588" y1="-3.429" x2="2.032" y2="-3.429" width="0.0508" layer="21"/>
<wire x1="2.032" y1="-3.429" x2="2.032" y2="-4.445" width="0.127" layer="21"/>
<wire x1="7.112" y1="-3.429" x2="11.684" y2="-3.429" width="0.0508" layer="21"/>
<wire x1="11.684" y1="-3.429" x2="11.684" y2="3.429" width="0.0508" layer="21"/>
<wire x1="11.684" y1="3.429" x2="-11.684" y2="3.429" width="0.0508" layer="21"/>
<wire x1="-11.684" y1="3.429" x2="-11.684" y2="-3.429" width="0.0508" layer="21"/>
<wire x1="-11.684" y1="-3.429" x2="-2.032" y2="-3.429" width="0.0508" layer="21"/>
<wire x1="-2.032" y1="-3.429" x2="-2.032" y2="-4.445" width="0.127" layer="21"/>
<wire x1="5.588" y1="-3.429" x2="5.588" y2="-3.937" width="0.127" layer="21"/>
<wire x1="5.588" y1="-3.937" x2="5.08" y2="-3.937" width="0.127" layer="21"/>
<wire x1="7.112" y1="-3.429" x2="7.112" y2="-3.937" width="0.127" layer="21"/>
<wire x1="7.112" y1="-3.937" x2="5.588" y2="-3.937" width="0.127" layer="21"/>
<wire x1="-2.032" y1="-4.445" x2="-6.858" y2="-4.445" width="0.127" layer="21"/>
<wire x1="-6.858" y1="-4.318" x2="-6.858" y2="-4.445" width="0.127" layer="21"/>
<wire x1="-6.858" y1="-4.318" x2="-8.382" y2="-4.318" width="0.127" layer="21"/>
<wire x1="-8.382" y1="-4.445" x2="-8.382" y2="-4.318" width="0.127" layer="21"/>
<wire x1="-8.382" y1="-4.445" x2="-12.7" y2="-4.445" width="0.127" layer="21"/>
<pad name="1" x="-7.62" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="2" x="-7.62" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="3" x="-5.08" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="4" x="-5.08" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="5" x="-2.54" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="6" x="-2.54" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="7" x="0" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="8" x="0" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="9" x="2.54" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="10" x="2.54" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="11" x="5.08" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="12" x="5.08" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="13" x="7.62" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="14" x="7.62" y="1.27" drill="0.9144" shape="octagon"/>
<text x="-12.7" y="5.08" size="1.778" layer="25" ratio="10">&gt;NAME</text>
<text x="0" y="5.08" size="1.778" layer="27" ratio="10">&gt;VALUE</text>
<text x="-1.016" y="-4.064" size="1.27" layer="21" ratio="10">14</text>
<text x="-10.16" y="-1.905" size="1.27" layer="21" ratio="10">1</text>
<text x="-10.16" y="0.635" size="1.27" layer="21" ratio="10">2</text>
<rectangle x1="7.366" y1="1.016" x2="7.874" y2="1.524" layer="51"/>
<rectangle x1="4.826" y1="1.016" x2="5.334" y2="1.524" layer="51"/>
<rectangle x1="4.826" y1="-1.524" x2="5.334" y2="-1.016" layer="51"/>
<rectangle x1="7.366" y1="-1.524" x2="7.874" y2="-1.016" layer="51"/>
<rectangle x1="-5.334" y1="1.016" x2="-4.826" y2="1.524" layer="51"/>
<rectangle x1="-7.874" y1="1.016" x2="-7.366" y2="1.524" layer="51"/>
<rectangle x1="-2.794" y1="1.016" x2="-2.286" y2="1.524" layer="51"/>
<rectangle x1="2.286" y1="1.016" x2="2.794" y2="1.524" layer="51"/>
<rectangle x1="-0.254" y1="1.016" x2="0.254" y2="1.524" layer="51"/>
<rectangle x1="-5.334" y1="-1.524" x2="-4.826" y2="-1.016" layer="51"/>
<rectangle x1="-7.874" y1="-1.524" x2="-7.366" y2="-1.016" layer="51"/>
<rectangle x1="-2.794" y1="-1.524" x2="-2.286" y2="-1.016" layer="51"/>
<rectangle x1="2.286" y1="-1.524" x2="2.794" y2="-1.016" layer="51"/>
<rectangle x1="-0.254" y1="-1.524" x2="0.254" y2="-1.016" layer="51"/>
</package>
</packages>
<symbols>
<symbol name="14P">
<wire x1="3.81" y1="-10.16" x2="-3.81" y2="-10.16" width="0.4064" layer="94"/>
<wire x1="-3.81" y1="10.16" x2="-3.81" y2="-10.16" width="0.4064" layer="94"/>
<wire x1="-3.81" y1="10.16" x2="3.81" y2="10.16" width="0.4064" layer="94"/>
<wire x1="3.81" y1="-10.16" x2="3.81" y2="10.16" width="0.4064" layer="94"/>
<wire x1="2.54" y1="5.08" x2="3.175" y2="5.08" width="0.1524" layer="94"/>
<wire x1="1.27" y1="2.54" x2="2.54" y2="2.54" width="0.6096" layer="94"/>
<wire x1="1.27" y1="0" x2="2.54" y2="0" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-2.54" x2="2.54" y2="-2.54" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-5.08" x2="2.54" y2="-5.08" width="0.6096" layer="94"/>
<wire x1="1.27" y1="-7.62" x2="2.54" y2="-7.62" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="-1.27" y2="2.54" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="0" x2="-1.27" y2="0" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-2.54" x2="-1.27" y2="-2.54" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-5.08" x2="-1.27" y2="-5.08" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="-7.62" x2="-1.27" y2="-7.62" width="0.6096" layer="94"/>
<wire x1="1.27" y1="5.08" x2="2.54" y2="5.08" width="0.6096" layer="94"/>
<wire x1="1.27" y1="7.62" x2="2.54" y2="7.62" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="5.08" x2="-1.27" y2="5.08" width="0.6096" layer="94"/>
<wire x1="-2.54" y1="7.62" x2="-1.27" y2="7.62" width="0.6096" layer="94"/>
<text x="-3.81" y="-12.7" size="1.778" layer="96">&gt;VALUE</text>
<text x="-3.81" y="10.922" size="1.778" layer="95">&gt;NAME</text>
<pin name="1" x="7.62" y="-7.62" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="3" x="7.62" y="-5.08" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="5" x="7.62" y="-2.54" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="7" x="7.62" y="0" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="9" x="7.62" y="2.54" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="11" x="7.62" y="5.08" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="13" x="7.62" y="7.62" visible="pad" length="middle" direction="pas" rot="R180"/>
<pin name="2" x="-7.62" y="-7.62" visible="pad" length="middle" direction="pas"/>
<pin name="4" x="-7.62" y="-5.08" visible="pad" length="middle" direction="pas"/>
<pin name="6" x="-7.62" y="-2.54" visible="pad" length="middle" direction="pas"/>
<pin name="12" x="-7.62" y="5.08" visible="pad" length="middle" direction="pas"/>
<pin name="14" x="-7.62" y="7.62" visible="pad" length="middle" direction="pas"/>
<pin name="8" x="-7.62" y="0" visible="pad" length="middle" direction="pas"/>
<pin name="10" x="-7.62" y="2.54" visible="pad" length="middle" direction="pas"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="ML14" prefix="SV" uservalue="yes">
<description>&lt;b&gt;HARTING&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="14P" x="0" y="0"/>
</gates>
<devices>
<device name="" package="ML14">
<connects>
<connect gate="1" pin="1" pad="1"/>
<connect gate="1" pin="10" pad="10"/>
<connect gate="1" pin="11" pad="11"/>
<connect gate="1" pin="12" pad="12"/>
<connect gate="1" pin="13" pad="13"/>
<connect gate="1" pin="14" pad="14"/>
<connect gate="1" pin="2" pad="2"/>
<connect gate="1" pin="3" pad="3"/>
<connect gate="1" pin="4" pad="4"/>
<connect gate="1" pin="5" pad="5"/>
<connect gate="1" pin="6" pad="6"/>
<connect gate="1" pin="7" pad="7"/>
<connect gate="1" pin="8" pad="8"/>
<connect gate="1" pin="9" pad="9"/>
</connects>
<technologies>
<technology name="">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="unknown" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="supply1">
<description>&lt;b&gt;Supply Symbols&lt;/b&gt;&lt;p&gt;
 GND, VCC, 0V, +5V, -5V, etc.&lt;p&gt;
 Please keep in mind, that these devices are necessary for the
 automatic wiring of the supply signals.&lt;p&gt;
 The pin name defined in the symbol is identical to the net which is to be wired automatically.&lt;p&gt;
 In this library the device names are the same as the pin names of the symbols, therefore the correct signal names appear next to the supply symbols in the schematic.&lt;p&gt;
 &lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="GND">
<wire x1="-1.905" y1="0" x2="1.905" y2="0" width="0.254" layer="94"/>
<text x="-2.54" y="-2.54" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="GND" prefix="GND">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="GND" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="X1" library="con-stewart" deviceset="SI-50170" device=""/>
<part name="SV1" library="con-ml" deviceset="ML14" device=""/>
<part name="GND1" library="supply1" deviceset="GND" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="X1" gate="G$1" x="17.78" y="66.04" rot="MR0"/>
<instance part="SV1" gate="1" x="73.66" y="63.5"/>
<instance part="GND1" gate="1" x="12.7" y="38.1"/>
</instances>
<busses>
</busses>
<nets>
<net name="GND" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="S1"/>
<wire x1="12.7" y1="48.26" x2="12.7" y2="40.64" width="0.1524" layer="91"/>
<pinref part="GND1" gate="1" pin="GND"/>
<pinref part="X1" gate="G$1" pin="S2"/>
<wire x1="10.16" y1="48.26" x2="10.16" y2="40.64" width="0.1524" layer="91"/>
<wire x1="10.16" y1="40.64" x2="12.7" y2="40.64" width="0.1524" layer="91"/>
</segment>
<segment>
<pinref part="X1" gate="G$1" pin="C_GND"/>
<wire x1="27.94" y1="63.5" x2="35.56" y2="63.5" width="0.1524" layer="91"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="12"/>
<wire x1="66.04" y1="68.58" x2="58.42" y2="68.58" width="0.1524" layer="91"/>
<label x="55.88" y="68.58" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="8"/>
<wire x1="66.04" y1="63.5" x2="58.42" y2="63.5" width="0.1524" layer="91"/>
<label x="55.88" y="63.5" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="7"/>
<wire x1="81.28" y1="63.5" x2="88.9" y2="63.5" width="0.1524" layer="91"/>
<label x="81.28" y="63.5" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="4"/>
<wire x1="66.04" y1="58.42" x2="58.42" y2="58.42" width="0.1524" layer="91"/>
<label x="55.88" y="58.42" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="X1" gate="G$1" pin="NC"/>
<wire x1="27.94" y1="66.04" x2="35.56" y2="66.04" width="0.1524" layer="91"/>
</segment>
</net>
<net name="TD+" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="TD+"/>
<wire x1="27.94" y1="81.28" x2="35.56" y2="81.28" width="0.1524" layer="91"/>
<label x="30.48" y="81.28" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="10"/>
<wire x1="66.04" y1="66.04" x2="58.42" y2="66.04" width="0.1524" layer="91"/>
<label x="55.88" y="66.04" size="1.778" layer="95"/>
</segment>
</net>
<net name="TCT" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="TCT"/>
<wire x1="27.94" y1="78.74" x2="35.56" y2="78.74" width="0.1524" layer="91"/>
<label x="30.48" y="78.74" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="11"/>
<wire x1="81.28" y1="68.58" x2="88.9" y2="68.58" width="0.1524" layer="91"/>
<label x="81.28" y="68.58" size="1.778" layer="95"/>
</segment>
</net>
<net name="TD-" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="TD-"/>
<wire x1="27.94" y1="76.2" x2="35.56" y2="76.2" width="0.1524" layer="91"/>
<label x="30.48" y="76.2" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="9"/>
<wire x1="81.28" y1="66.04" x2="88.9" y2="66.04" width="0.1524" layer="91"/>
<label x="81.28" y="66.04" size="1.778" layer="95"/>
</segment>
</net>
<net name="RD+" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="RD+"/>
<wire x1="27.94" y1="73.66" x2="35.56" y2="73.66" width="0.1524" layer="91"/>
<label x="30.48" y="73.66" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="6"/>
<wire x1="66.04" y1="60.96" x2="58.42" y2="60.96" width="0.1524" layer="91"/>
<label x="55.88" y="60.96" size="1.778" layer="95"/>
</segment>
</net>
<net name="RD-" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="RD-"/>
<wire x1="27.94" y1="68.58" x2="35.56" y2="68.58" width="0.1524" layer="91"/>
<label x="30.48" y="68.58" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="5"/>
<wire x1="81.28" y1="60.96" x2="88.9" y2="60.96" width="0.1524" layer="91"/>
<label x="81.28" y="60.96" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEDA" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="A"/>
<wire x1="27.94" y1="60.96" x2="35.56" y2="60.96" width="0.1524" layer="91"/>
<label x="30.48" y="60.96" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="1"/>
<wire x1="81.28" y1="55.88" x2="88.9" y2="55.88" width="0.1524" layer="91"/>
<label x="81.28" y="55.88" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEDAC" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="C"/>
<wire x1="27.94" y1="58.42" x2="35.56" y2="58.42" width="0.1524" layer="91"/>
<label x="30.48" y="58.42" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="2"/>
<wire x1="66.04" y1="55.88" x2="58.42" y2="55.88" width="0.1524" layer="91"/>
<label x="55.88" y="55.88" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEDB" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="A1"/>
<wire x1="27.94" y1="55.88" x2="35.56" y2="55.88" width="0.1524" layer="91"/>
<label x="30.48" y="55.88" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="13"/>
<wire x1="81.28" y1="71.12" x2="88.9" y2="71.12" width="0.1524" layer="91"/>
<label x="81.28" y="71.12" size="1.778" layer="95"/>
</segment>
</net>
<net name="LEDBC" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="C1"/>
<wire x1="27.94" y1="53.34" x2="35.56" y2="53.34" width="0.1524" layer="91"/>
<label x="30.48" y="53.34" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="14"/>
<wire x1="66.04" y1="71.12" x2="58.42" y2="71.12" width="0.1524" layer="91"/>
<label x="55.88" y="71.12" size="1.778" layer="95"/>
</segment>
</net>
<net name="RCT" class="0">
<segment>
<pinref part="X1" gate="G$1" pin="RCT"/>
<wire x1="27.94" y1="71.12" x2="35.56" y2="71.12" width="0.1524" layer="91"/>
<label x="30.48" y="71.12" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="SV1" gate="1" pin="3"/>
<wire x1="81.28" y1="58.42" x2="88.9" y2="58.42" width="0.1524" layer="91"/>
<label x="81.28" y="58.42" size="1.778" layer="95"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>

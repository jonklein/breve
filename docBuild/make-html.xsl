<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:import href="/usr/local/lib/docbook-xsl-1.70.1/html/chunk.xsl"/>
<xsl:include href="titlepages.xsl"/>

<xsl:param name="base.dir" select="'htmldocs/'"/>
<xsl:param name="html.stylesheet" select="'stylesheet.css'"/>

<xsl:param name="section.autolabel" select="0"/>
<xsl:param name="section.label.includes.component.label" select="0"/>

<xsl:param name="xref.with.number.and.title" select="0"/>

<xsl:param name="navig.graphics">../images/</xsl:param>
<xsl:param name="navig.graphics.path">../images/</xsl:param>
<xsl:param name="navig.graphics.extension" select="'.gif'"/>

<xsl:param name="admon.graphics" select="1"/>
<xsl:param name="admon.graphics.path">../images/</xsl:param>
<xsl:param name="admon.graphics.extension" select="'.gif'"/>

<xsl:param name="generate.index" select="1"></xsl:param>
<xsl:param name="index.term.separator" select="'::'"></xsl:param>

<xsl:param name="local.l10n.xml" select="document('')" />
<l:i18n xmlns:l="http://docbook.sourceforge.net/xmlns/l10n/1.0">
 <l:l10n language="en">
  <l:gentext key="nav-next" text="next >>"/>
  <l:gentext key="nav-prev" text="&lt;&lt; previous"/> 
  <l:gentext key="nav-home" text="Table of Contents"/>
 </l:l10n>
</l:i18n>

<xsl:param name="toc.section.depth" select="'1'"/>
<xsl:param name="generate.toc">
appendix  toc,title
article/appendix  nop
article   toc,title
book      toc,title,figure,table,example,equation
chapter   title
part      toc,title
preface   title
qandadiv  toc
qandaset  toc
reference toc,title
sect1     nop
sect2     nop
sect3     nop
sect4     nop
sect5     nop
section   toc
set       toc,title
</xsl:param>
</xsl:stylesheet>

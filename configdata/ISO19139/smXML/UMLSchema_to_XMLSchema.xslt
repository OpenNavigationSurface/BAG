<?xml version="1.0" encoding="UTF-8"?>
<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<!-- NEWLINE = &#xA; -->
	<xsl:template match="/">
		<xsl:text>&#xA;</xsl:text>
		<xs:schema targetNamespace="http://metadata.dgiwg.org/smXML/" xmlns:smXML="http://metadata.dgiwg.org/smXML/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:gml="http://www.opengis.net/gml" version="0.8">
			<xsl:call-template name="SchemaDefinition"/>
		</xs:schema>
	</xsl:template>
	<xsl:template name="SchemaDefinition">
		<xsl:call-template name="Separator"/>
		<xsl:element name="xs:annotation">
			<xsl:element name="xs:documentation">
	    			This schema was directly generated from the ISO 19139 UML model via
	    			Version 0.8 of the Rational Rose Script 'XMLout4XMLSchema.ebs' and via
	    			'UMLSchema_to_XMLSchema.xslt' for the DGIWG contribution to the ISO 19139 Project Team
	    			 at DateTime <xsl:value-of select="//date"/>
			</xsl:element>
		</xsl:element>
		<xsl:call-template name="Separator"/>
		<xsl:comment> ================== Imports ================== </xsl:comment>
		<xsl:call-template name="Separator"/>
		<xsl:call-template name="Separator"/>
		<xsl:comment> ================== Classes  ================== </xsl:comment>
		<xsl:for-each select="classList">
			<!--================== class START  ==================-->
			<xsl:for-each select="class">
				<xsl:for-each select="restriction">
					<xsl:call-template name="Separator"/>
					<xsl:element name="xs:complexType">
						<xsl:attribute name="name"><xsl:value-of select="type"/></xsl:attribute>
						<xsl:attribute name="abstract">true</xsl:attribute>
						<xsl:element name="xs:complexContent">
							<xsl:element name="xs:restriction">
								<xsl:attribute name="base"><xsl:value-of select="restrictionOf"/></xsl:attribute>
								<xsl:for-each select="classList">
									<xsl:element name="xs:sequence">
										<xsl:for-each select="class">
											<xsl:call-template name="ElementsForComplexType"/>
										</xsl:for-each>
										<!-- class -->
									</xsl:element>
									<xsl:for-each select="class">
										<xsl:call-template name="AttributesForComplexType"/>
									</xsl:for-each>
									<!-- class -->
								</xsl:for-each>
								<!-- classList -->
							</xsl:element>
						</xsl:element>
					</xsl:element>
				</xsl:for-each>
				<!--end of restriction -->
				<xsl:call-template name="Separator"/>
				<xsl:element name="xs:complexType">
					<xsl:variable name="className" select="name"/>
					<xsl:variable name="classType" select="type"/>
					<!--tmt:  If the type name has 'smXML:' pre-pended to it, output the name + "Type" only  -->
					<xsl:choose>
						<xsl:when test="string-length($classType) &gt; 6 and substring($classType, 1, 6)  =  'smXML:'">
							<xsl:attribute name="name"><xsl:value-of select="$className"/>_Type</xsl:attribute>
						</xsl:when>
						<xsl:otherwise>
							<xsl:attribute name="name"><xsl:value-of select="$classType"/></xsl:attribute>
						</xsl:otherwise>
					</xsl:choose>
					<xsl:if test="isAbstract">
						<xsl:attribute name="abstract">true</xsl:attribute>
					</xsl:if>
					<xsl:choose>
						<xsl:when test="extensionOf">
							<xsl:element name="xs:complexContent">
								<xsl:element name="xs:extension">
									<xsl:attribute name="base"><xsl:value-of select="extensionOf"/></xsl:attribute>
									<xsl:if test="notEmpty">
										<xsl:element name="xs:sequence">
											<xsl:call-template name="ElementsForComplexType"/>
										</xsl:element>
									</xsl:if>
									<xsl:call-template name="AttributesForComplexType"/>
								</xsl:element>
							</xsl:element>
						</xsl:when>
						<!--  test="extensionOf" -->
						<xsl:otherwise>
							<xsl:if test="notEmpty">
								<xsl:element name="xs:sequence">
									<xsl:call-template name="ElementsForComplexType"/>
								</xsl:element>
								<!-- ljb-working on XCT -->
							</xsl:if>
							<xsl:call-template name="AttributesForComplexType"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:element>
				<xsl:call-template name="GlobalElementDefinition"/>
				<!-- ljb: v0.8 added XCPT declaration -->
				<xsl:element name="xs:complexType">
					<xsl:variable name="className" select="name"/>
					<xsl:variable name="classType" select="type"/>
					<!-- ljb: v0.8  If the type name has 'smXML:' pre-pended to it, output the name + "Type" only  -->
					<xsl:if test="string-length($classType) &gt; 6 and substring($classType, 1, 6)  =  'smXML:'">
						<xsl:attribute name="name"><xsl:value-of select="$className"/>_PropertyType</xsl:attribute>
						<xsl:element name="xs:sequence">
							<xsl:element name="xs:element">
								<xsl:attribute name="ref"><xsl:value-of select="$className"/></xsl:attribute>
								<!-- ljb: v0.8 TODO - check where the value of minOccurs actually comes from-->
								<xsl:attribute name="minOccurs">0</xsl:attribute>
							</xsl:element>
						</xsl:element>
						<xsl:element name="xs:attributeGroup">
							<xsl:attribute name="ref">smXML:simpleLink</xsl:attribute>
						</xsl:element>
					</xsl:if>
				</xsl:element>
				<!--end of XCPT declaration -->
			</xsl:for-each>
			<!-- class -->
			<!--================== class END  ==================-->
			<!--================== enumeration START  ==================-->
			<!-- If useDomainCodes == 'yes', then use 'xs:complexType' with xs:choice of either xs:simpleType (string) or
			     xs:integer.  Else, if useDomainCodes == 'no',  just use xs:simpleType (string)  -->
			     
<!--0.8 tf Enumerations will now be encoded just like codelists.  Left this in so you could see the changes.

		<xsl:for-each select="enumeration">
				<xsl:variable name="enumName" select="name"/>
				<xsl:variable name="enumType" select="type"/>
				<xsl:variable name="useDomainCodes" select="useDomainCodes"/>
				<xsl:variable name="numCodes" select="count(simpleAttribute) + count(basicAttribute) + count(attribute)"/>
				<xsl:call-template name="Separator"/>
				<xsl:choose>
	
					<xsl:when test="$useDomainCodes = 'yes'">
						<xsl:element name="xs:complexType">
							<xsl:choose>
								<xsl:when test="string-length($enumType) &gt; 6 and substring($enumType, 1, 6)  =  'smXML:'">
									<xsl:attribute name="name"><xsl:value-of select="$enumName"/>Type</xsl:attribute>
								</xsl:when>
								<xsl:otherwise>
									<xsl:attribute name="name"><xsl:value-of select="$enumType"/></xsl:attribute>
								</xsl:otherwise>
							</xsl:choose>
							<xs:choice>
								<xsl:element name="xs:element">
									<xsl:attribute name="name"><xsl:value-of select="$enumName"/>_CodeList</xsl:attribute>
									<xsl:element name="xs:simpleType">
										<xsl:element name="xs:restriction">
											<xsl:attribute name="base">iso19103:CharacterString</xsl:attribute>
											<xsl:for-each select="basicAttribute">
												<xsl:element name="xs:enumeration">
													<xsl:attribute name="value"><xsl:value-of select="name"/></xsl:attribute>
												</xsl:element>
											</xsl:for-each>
											<xsl:for-each select="simpleAttribute">
												<xsl:element name="xs:enumeration">
													<xsl:attribute name="value"><xsl:value-of select="name"/></xsl:attribute>
												</xsl:element>
											</xsl:for-each>
											<xsl:for-each select="attribute">
												<xsl:element name="xs:enumeration">
													<xsl:attribute name="value"><xsl:value-of select="name"/></xsl:attribute>
												</xsl:element>
											</xsl:for-each>
										</xsl:element>
									</xsl:element>
								</xsl:element>
								<xsl:element name="xs:element">
									<xsl:attribute name="name"><xsl:value-of select="$enumName"/>_DomainCodes</xsl:attribute>
									<xsl:element name="xs:simpleType">
										<xsl:element name="xs:restriction">
											<xsl:attribute name="base">xs:positiveInteger</xsl:attribute>
											<xsl:element name="xs:maxInclusive">
												<xsl:attribute name="value"><xsl:value-of select="$numCodes"/></xsl:attribute>
											</xsl:element>
											<xsl:element name="xs:pattern">
												<xsl:attribute name="value">\d{3}</xsl:attribute>
											</xsl:element>
										</xsl:element>
									</xsl:element>
								</xsl:element>
							</xs:choice>
						</xsl:element>
					</xsl:when>

					<xsl:otherwise>
						<xsl:element name="xs:simpleType">
							<xsl:choose>
								<xsl:when test="string-length($enumType) &gt; 6 and substring($enumType, 1, 6)  =  'smXML:'">
									<xsl:attribute name="name"><xsl:value-of select="$enumName"/>Type</xsl:attribute>
								</xsl:when>
								<xsl:otherwise>
									<xsl:attribute name="name"><xsl:value-of select="$enumType"/></xsl:attribute>
								</xsl:otherwise>

							</xsl:choose>
							<xsl:element name="xs:restriction">
								<xsl:attribute name="base">iso19103:CharacterString</xsl:attribute>
								<xsl:for-each select="basicAttribute">
									<xsl:element name="xs:enumeration">
										<xsl:attribute name="value"><xsl:value-of select="name"/></xsl:attribute>
									</xsl:element>
								</xsl:for-each>
								<xsl:for-each select="simpleAttribute">
									<xsl:element name="xs:enumeration">
										<xsl:attribute name="value"><xsl:value-of select="name"/></xsl:attribute>
									</xsl:element>
								</xsl:for-each>
								<xsl:for-each select="attribute">
									<xsl:element name="xs:enumeration">
										<xsl:attribute name="value"><xsl:value-of select="name"/></xsl:attribute>
									</xsl:element>
								</xsl:for-each>
							</xsl:element>
						</xsl:element>
					</xsl:otherwise>
				</xsl:choose>
				<xsl:call-template name="GlobalElementDefinition"/>
			</xsl:for-each>
-->
			<!--================== enumeration END  ==================-->
			
			<!--==================codeList and enumeration START  ==================-->
			<!-- 0.8 tf adapted from previous codelist -->
			<!--most of the info in the template is ignored since iso19118 specifies
			that codelists should reside outside of the schema  -->
			<xsl:for-each select="codeList | enumeration">
				<xsl:variable name="enumName" select="name"/>
				<xsl:variable name="enumType" select="type"/>
				<xsl:text> &#xD; </xsl:text>
				<!--write out the complexType definition -->
					
				<xsl:element name="xs:complexType">
					<!-- If the type name has 'smXML:' pre-pended to it, output the name only  -->
					<xsl:choose>
						<xsl:when test="string-length($enumType) &gt; 6 and substring($enumType, 1, 6)  =  'smXML:'">
							<xsl:attribute name="name"><xsl:value-of select="$enumName"/>_Type</xsl:attribute>
						</xsl:when>
						<xsl:otherwise>
							<xsl:attribute name="name"><xsl:value-of select="$enumType"/>_Type</xsl:attribute>
						</xsl:otherwise>
					</xsl:choose>
					<xsl:element name="xs:simpleContent">
						<xsl:element name="xs:extension">
							<xsl:attribute name="base"><xsl:value-of select="'iso19103:CharacterString'"/></xsl:attribute>
							<xsl:element name="xs:attribute">
								<xsl:attribute name="name"><xsl:value-of select="'codeSpace'"/></xsl:attribute>
								<xsl:attribute name="type"><xsl:value-of select="'xs:anyURI'"/></xsl:attribute>
								<xsl:attribute name="use"><xsl:value-of select="'optional'"/></xsl:attribute>
							</xsl:element>
						</xsl:element>
					</xsl:element>
				</xsl:element>
				<!--write out the actual element definition -->
				<xsl:text> &#xD;</xsl:text>
				<xsl:call-template name="GlobalElementDefinition"/>
					
			</xsl:for-each>
			<!--================== codeList END  ==================-->
			
			
			<!--================== union START  ==================-->
			<xsl:for-each select="union">
				<xsl:call-template name="Separator"/>
				<xsl:element name="xs:complexType">
					<xsl:attribute name="name"><xsl:value-of select="name"/>_Type</xsl:attribute>
					<xsl:element name="xs:choice">
						<xsl:for-each select="attribute">
							<xsl:element name="xs:element">
								<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
								<xsl:attribute name="type"><xsl:value-of select="type"/></xsl:attribute>
								<xsl:if test="cardinality/min != 0 and cardinality/min !=1">
									<xsl:attribute name="minOccurs"><xsl:value-of select="cardinality/min"/></xsl:attribute>
								</xsl:if>
								<xsl:if test="cardinality/max != 1">
									<xsl:attribute name="maxOccurs"><xsl:value-of select="cardinality/max"/></xsl:attribute>
								</xsl:if>
							</xsl:element>
						</xsl:for-each>
						<xsl:for-each select="basicAttribute">
							<xsl:element name="xs:element">
								<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
								<xsl:attribute name="type"><xsl:value-of select="type"/></xsl:attribute>
							</xsl:element>
						</xsl:for-each>
						<xsl:for-each select="simpleAttribute">
							<xsl:call-template name="ElementDefinition"/>
						</xsl:for-each>
					</xsl:element>
				</xsl:element>
<!--				<xsl:call-template name="ElementDefinition"/> -->
			</xsl:for-each>
			<!--================== union END  ==================-->
			<!--================== array START  =========================-->
			<xsl:for-each select="array">
				<xsl:call-template name="Separator"/>
				<xsl:element name="xs:complexType">
					<xsl:attribute name="name"><xsl:value-of select="type"/></xsl:attribute>
					<xsl:if test="isAbstract">
						<xsl:attribute name="abstract">true</xsl:attribute>
					</xsl:if>
					<xsl:choose>
						<xsl:when test="superclass">
							<xsl:element name="xs:complexContent">
								<xsl:element name="xs:restriction">
									<xsl:attribute name="base"><xsl:value-of select="type"/></xsl:attribute>
									<xsl:if test="notEmpty">
										<xsl:element name="xs:sequence">
											<xsl:call-template name="ElementsForArrayType"/>
										</xsl:element>
									</xsl:if>
									<xsl:call-template name="AttributesForArrayType"/>
								</xsl:element>
							</xsl:element>
						</xsl:when>
						<!--  test="extensionOf" -->
						<xsl:otherwise>
							<xsl:if test="notEmpty">
								<xsl:element name="xs:sequence">
									<xsl:call-template name="ElementsForArrayType"/>
								</xsl:element>
								<!-- <xsl:for-each select="notEmpty">&lt;/xs:sequence&gt;</xsl:for-each> -->
							</xsl:if>
							<xsl:call-template name="AttributesForArrayType"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:element>
				<xsl:call-template name="GlobalElementDefinition"/>
			</xsl:for-each>
			<!--================== array END  =========================-->
			<!--================== simpleType START  ==================-->
			<xsl:for-each select="simpleType">
				<xsl:call-template name="Separator"/>
				<xsl:element name="xs:simpleType">
					<!--
					<xsl:attribute name="name"><xsl:value-of select="type"/></xsl:attribute>
				-->
					<xsl:variable name="className" select="name"/>
					<xsl:variable name="classType" select="type"/>
					<!--tmt:  If the type name has 'smXML:' pre-pended to it, output the name + "Type" only  -->
					<xsl:choose>
						<xsl:when test="string-length($classType) &gt; 6 and substring($classType, 1, 6)  =  'smXML:'">
							<xsl:attribute name="name"><xsl:value-of select="$className"/>_Type</xsl:attribute>
						</xsl:when>
						<xsl:otherwise>
							<xsl:attribute name="name"><xsl:value-of select="$classType"/></xsl:attribute>
						</xsl:otherwise>
					</xsl:choose>
					<xsl:if test="restrictBase">
						<xsl:element name="xs:restriction">
							<xsl:attribute name="base"><xsl:value-of select="restrictBase"/></xsl:attribute>
							<xsl:if test="minLength">
								<xsl:element name="xs:minLength">
									<xsl:attribute name="value"><xsl:value-of select="minLength"/></xsl:attribute>
								</xsl:element>
							</xsl:if>
							<xsl:if test="minIncl">
								<xsl:element name="xs:minInclusive">
									<xsl:attribute name="value"><xsl:value-of select="minIncl"/></xsl:attribute>
								</xsl:element>
							</xsl:if>
							<xsl:if test="minExcl">
								<xsl:element name="xs:minExclusive">
									<xsl:attribute name="value"><xsl:value-of select="minExcl"/></xsl:attribute>
								</xsl:element>
							</xsl:if>
							<xsl:if test="maxIncl">
								<xsl:element name="xs:maxInclusive">
									<xsl:attribute name="value"><xsl:value-of select="maxIncl"/></xsl:attribute>
								</xsl:element>
							</xsl:if>
							<xsl:if test="maxExcl">
								<xsl:element name="xs:maxExclusive">
									<xsl:attribute name="value"><xsl:value-of select="maxExcl"/></xsl:attribute>
								</xsl:element>
							</xsl:if>
							<xsl:if test="fractionDigits">
								<xsl:element name="xs:fractionDigits">
									<xsl:attribute name="value"><xsl:value-of select="fractionDigits"/></xsl:attribute>
								</xsl:element>
							</xsl:if>
						</xsl:element>
					</xsl:if>
				</xsl:element>
				<!--<xsl:call-template name="ElementDefinition"/>-->
			</xsl:for-each>
			<!--================== simpleType END  ==================-->
		</xsl:for-each>
		<!-- classList -->
		<!--================== classList END  ==================-->
	</xsl:template>
	<!-- SchemaDefinition -->
	<xsl:template match="name">
		<xsl:apply-templates/>
	</xsl:template>
	<xsl:template match="type">
		<xsl:apply-templates/>
	</xsl:template>
	<xsl:template name="ElementsForComplexType">
		<!--
		<xsl:for-each select="attribute">
			<xsl:call-template name="LocalElementDefinition"/>
			
			
			
			
		</xsl:for-each>
-->
		<xsl:for-each select="basicAttribute | attribute">
			<xsl:sort select="@position" data-type="number"/>
			<xsl:sort select="name()"/>
			<xsl:call-template name="LocalElementDefinition"/>
		</xsl:for-each>
		
		<!-- 0.8 tf added for Role implementation per Nicolas' Encoding -->
		<xsl:for-each select="role">
			<xsl:sort select="@position" data-type="number"/>
			<xsl:sort select="name()"/>
			<xsl:call-template name="LocalRoleDefinition"/>
		</xsl:for-each>

	</xsl:template>
	<xsl:template name="ElementsForArrayType">
		<xsl:for-each select="attribute">
			<xsl:call-template name="ElementReference"/>
		</xsl:for-each>
		<xsl:for-each select="longAttribute">
			<xsl:call-template name="ElementReference"/>
		</xsl:for-each>
		<xsl:for-each select="longTargetrole">
			<xsl:call-template name="ElementReference"/>
		</xsl:for-each>
		<xsl:for-each select="basicAttribute">
			<xsl:call-template name="ElementReference"/>
		</xsl:for-each>
		<xsl:for-each select="longBasicAttribute">
			<xsl:call-template name="ElementReference"/>
		</xsl:for-each>
		<xsl:for-each select="targetrole">
			<!-- I don't see any of these ... -->
			<xsl:call-template name="ElementReference"/>
		</xsl:for-each>
	</xsl:template>
	<!-- START of TEMPLATE: LocalElementDefinition -->
	<xsl:template name="LocalElementDefinition">
		<xsl:variable name="numValConstraints" select="count(minIncl) + count(minExcl) + count(maxIncl) + count(maxExcl)"/>
		<xsl:element name="xs:element">
			<xsl:variable name="typeValue">
				<xsl:value-of select="substring-before(type,':_')"/>
			</xsl:variable>
			<xsl:choose>
				<xsl:when test="$typeValue = 'smXML'">
					<xsl:attribute name="ref"><xsl:value-of select="$typeValue"/><xsl:text>:</xsl:text><xsl:value-of select="element"/></xsl:attribute>
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
					<xsl:if test="$numValConstraints = 0">
						<!--ljb:  v0.8 If the type name has 'smXML:' pre-pended to it, output the name + "_PropertyType" only  -->
						<xsl:variable name="propertyType">
							<xsl:value-of select="type"/>
						</xsl:variable>
						<xsl:choose>
							<xsl:when test="string-length($propertyType) &gt; 6 and substring($propertyType, 1, 6)  =  'smXML:'">
								<xsl:attribute name="type"><xsl:value-of select="type"/>_PropertyType</xsl:attribute>
							</xsl:when>
							<xsl:otherwise>
								<xsl:attribute name="type"><xsl:value-of select="type"/></xsl:attribute>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:if>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:if test="cardinality/min != '1'">
				<xsl:attribute name="minOccurs"><xsl:value-of select="cardinality/min"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="cardinality/max != '1'">
				<xsl:attribute name="maxOccurs"><xsl:value-of select="cardinality/max"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="default">
				<xsl:attribute name="default"><xsl:value-of select="default"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="static/default">
				<xsl:attribute name="fixed"><xsl:value-of select="static/default"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="$numValConstraints != 0">
				<xsl:element name="xs:simpleType">
					<xsl:element name="xs:restriction">
						<xsl:attribute name="base"><xsl:value-of select="type"/></xsl:attribute>
						<xsl:if test="minIncl">
							<xsl:element name="xs:minInclusive">
								<xsl:attribute name="value"><xsl:value-of select="minIncl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
						<xsl:if test="minExcl">
							<xsl:element name="xs:minExclusive">
								<xsl:attribute name="value"><xsl:value-of select="minExcl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
						<xsl:if test="maxIncl">
							<xsl:element name="xs:maxInclusive">
								<xsl:attribute name="value"><xsl:value-of select="maxIncl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
						<xsl:if test="maxExcl">
							<xsl:element name="xs:maxExclusive">
								<xsl:attribute name="value"><xsl:value-of select="maxExcl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
					</xsl:element>
				</xsl:element>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<!--END of TEMPLATE: LocalElementDefinition -->
	
	<!-- START of TEMPLATE: LocalRoleDefinition -->
	<!-- 0.8 tf added for Role implementation per Nicolas' Encoding -->
	<xsl:template name="LocalRoleDefinition">
		<xsl:element name="xs:element">
			<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
				<!--ljb:  v0.8 If the type name has 'smXML:' pre-pended to it, output the name + "_PropertyType" only  -->
				<xsl:variable name="propertyType">
					<xsl:value-of select="type"/>
				</xsl:variable>
				<xsl:choose>
					<xsl:when test="string-length($propertyType) &gt; 6 and substring($propertyType, 1, 6)  =  'smXML:'">
						<xsl:attribute name="type"><xsl:value-of select="type"/>_PropertyType</xsl:attribute>
					</xsl:when>
					<xsl:otherwise>
						<xsl:attribute name="type"><xsl:value-of select="type"/></xsl:attribute>
					</xsl:otherwise>
				</xsl:choose>

			<xsl:if test="cardinality/min != '1'">
				<xsl:attribute name="minOccurs"><xsl:value-of select="cardinality/min"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="cardinality/max != '1'">
				<xsl:attribute name="maxOccurs"><xsl:value-of select="cardinality/max"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="default">
				<xsl:attribute name="default"><xsl:value-of select="default"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="static/default">
				<xsl:attribute name="fixed"><xsl:value-of select="static/default"/></xsl:attribute>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<!--END of TEMPLATE: LocalRoleDefinition -->
	
	<xsl:template name="RepTarget">
		<xsl:element name="xs:element">
			<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
			<xsl:attribute name="type"><xsl:value-of select="representation"/></xsl:attribute>
			<xsl:if test="cardinality/min != 1">
				<xsl:attribute name="minOccurs"><xsl:value-of select="cardinality/min"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="cardinality/max != 1">
				<xsl:attribute name="maxOccurs"><xsl:value-of select="cardinality/max"/></xsl:attribute>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<xsl:template name="RefTarget">
		<xsl:element name="xs:element">
			<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
			<xsl:attribute name="type"><xsl:value-of select="reference"/></xsl:attribute>
			<xsl:if test="cardinality/min != 1">
				<xsl:attribute name="minOccurs"><xsl:value-of select="cardinality/min"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="cardinality/max != 1">
				<xsl:attribute name="maxOccurs"><xsl:value-of select="cardinality/max"/></xsl:attribute>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<xsl:template name="AttributesForComplexType">
		<xsl:for-each select="simpleAttribute">
			<xsl:call-template name="AttributeDefinition"/>
		</xsl:for-each>
		<xsl:for-each select="enumeration">
			<xsl:call-template name="AttributeDefinition"/>
		</xsl:for-each>
		<xsl:for-each select="longEnumeration">
			<xsl:element name="xs:attribute">
				<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
				<xsl:if test="cardinality/use">
					<xsl:attribute name="use"><xsl:value-of select="cardinality/use"/></xsl:attribute>
				</xsl:if>
				<xsl:element name="xs:simpleType">
					<xsl:element name="xs:list">
						<xsl:attribute name="itemType"><xsl:value-of select="type"/></xsl:attribute>
					</xsl:element>
				</xsl:element>
			</xsl:element>
		</xsl:for-each>
		<xsl:for-each select="longSimpleAttribute">
			<xsl:element name="xs:attribute">
				<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
				<xsl:if test="cardinality/use">
					<xsl:attribute name="use"><xsl:value-of select="cardinality/use"/></xsl:attribute>
				</xsl:if>
				<xsl:element name="xs:simpleType">
					<xsl:element name="xs:list">
						<xsl:attribute name="itemType"><xsl:value-of select="type"/></xsl:attribute>
					</xsl:element>
				</xsl:element>
			</xsl:element>
		</xsl:for-each>
		<xsl:for-each select="attributeGroup">
			<xsl:element name="xs:attributeGroup">
				<xsl:attribute name="ref"><xsl:value-of select="type"/></xsl:attribute>
			</xsl:element>
		</xsl:for-each>
		<!-- ljb: v0.8 adding the smXML:id -->
		<!-- ljb: v0.8 TODO - check for exceptions when the complexTypes don't get an smXML:id-->
		<xsl:element name="xs:attribute">
			<xsl:attribute name="ref">smXML:id</xsl:attribute>
			<xsl:attribute name="use">optional</xsl:attribute>
		</xsl:element>
	</xsl:template>
	<xsl:template name="AttributesForArrayType">
		<xsl:for-each select="simpleAttribute">
			<xsl:call-template name="AttributeDefinition"/>
		</xsl:for-each>
		<xsl:for-each select="longSimpleAttribute">
			<xsl:element name="xs:attribute">
				<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
				<xsl:element name="xs:simpleType">
					<xsl:element name="xs:list">
						<xsl:attribute name="itemType"><xsl:value-of select="type"/></xsl:attribute>
					</xsl:element>
				</xsl:element>
			</xsl:element>
		</xsl:for-each>
	</xsl:template>
	<xsl:template name="AttributeDefinition">
		<xsl:variable name="numValConstraints" select="count(minIncl) + count(minExcl) + count(maxIncl) + count(maxExcl)"/>
		<xsl:element name="xs:attribute">
			<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
			<xsl:if test="$numValConstraints = 0">
				<xsl:attribute name="type"><xsl:value-of select="type"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="cardinality/use">
				<xsl:attribute name="use"><xsl:value-of select="cardinality/use"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="default">
				<xsl:attribute name="default"><xsl:value-of select="default"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="static/default">
				<xsl:attribute name="fixed"><xsl:value-of select="static/default"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="$numValConstraints != 0">
				<xsl:element name="xs:simpleType">
					<xsl:element name="xs:restriction">
						<xsl:attribute name="base"><xsl:value-of select="type"/></xsl:attribute>
						<xsl:if test="minIncl">
							<xsl:element name="xs:minInclusive">
								<xsl:attribute name="value"><xsl:value-of select="minIncl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
						<xsl:if test="minExcl">
							<xsl:element name="xs:minExclusive">
								<xsl:attribute name="value"><xsl:value-of select="minExcl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
						<xsl:if test="maxIncl">
							<xsl:element name="xs:maxInclusive">
								<xsl:attribute name="value"><xsl:value-of select="maxIncl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
						<xsl:if test="maxExcl">
							<xsl:element name="xs:maxExclusive">
								<xsl:attribute name="value"><xsl:value-of select="maxExcl"/></xsl:attribute>
							</xsl:element>
						</xsl:if>
					</xsl:element>
				</xsl:element>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<xsl:template name="ElementReference">
		<xsl:element name="xs:element">
			<xsl:attribute name="ref"><xsl:value-of select="element"/></xsl:attribute>
			<xsl:if test="cardinality/min != 1">
				<xsl:attribute name="minOccurs"><xsl:value-of select="cardinality/min"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="cardinality/max != 1">
				<xsl:attribute name="maxOccurs"><xsl:value-of select="cardinality/max"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="default">
				<xsl:attribute name="default"><xsl:value-of select="default"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="static/default">
				<xsl:attribute name="fixed"><xsl:value-of select="static/default"/></xsl:attribute>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<xsl:template name="GlobalElementDefinition">
		<xsl:element name="xs:element">
			<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
			<!--ljb: v0.8 added support for attribute using _Type if part of smXML -->
			<xsl:variable name="elementType" select="type"/>
			<xsl:choose>
				<xsl:when test="string-length($elementType) &gt; 6 and substring($elementType, 1, 6)  =  'smXML:'">
					<xsl:attribute name="type"><xsl:value-of select="$elementType"/>_Type</xsl:attribute>
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="type"><xsl:value-of select="$elementType"/></xsl:attribute>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:if test="superclass">
				<xsl:variable name="scName" select="superclass/name"/>
				<xsl:variable name="scType" select="superclass/type"/>
				<xsl:choose>
					<xsl:when test="string-length($scType) &gt; 6 and substring($scType, 1, 6)  =  'smXML:'">
						<xsl:attribute name="substitutionGroup">smXML:<xsl:value-of select="$scName"/>_Type</xsl:attribute>
					</xsl:when>
					<xsl:otherwise>
						<xsl:attribute name="substitutionGroup"><xsl:value-of select="$scName"/></xsl:attribute>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:if>
			<xsl:if test="isAbstract">
				<xsl:attribute name="abstract">true</xsl:attribute>
			</xsl:if>
			<!-- tmt:21Mar02 Add annotation for class -->
			<xsl:if test="co-constraint">
				<xsl:element name="xs:annotation">
					<xsl:element name="xs:documentation">
						<xsl:for-each select="co-constraint">
							<xsl:text>&#xA;</xsl:text>
							<xsl:text>   ***   </xsl:text>
							<xsl:value-of select="."/>
						</xsl:for-each>
						<xsl:text>&#xA;</xsl:text>
					</xsl:element>
				</xsl:element>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<xsl:template name="GlobalArrayElementDefinition">
		<xsl:element name="xs:element">
			<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
			<xsl:attribute name="type"><xsl:value-of select="name"/>Type</xsl:attribute>
			<xsl:if test="superclass">
				<xsl:attribute name="substitutionGroup"><xsl:value-of select="superclass/name"/></xsl:attribute>
			</xsl:if>
			<xsl:if test="isAbstract">
				<xsl:attribute name="abstract">true</xsl:attribute>
			</xsl:if>
		</xsl:element>
	</xsl:template>
	<xsl:template name="ElementDefinition">
		<xsl:element name="xs:element">
			<xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute>
			<xsl:attribute name="type"><xsl:value-of select="type"/></xsl:attribute>
		</xsl:element>
	</xsl:template>
	<xsl:template name="Separator">
		<xsl:text disable-output-escaping="yes">
			<!-- =========================================== -->
		</xsl:text>
	</xsl:template>
</xsl:transform>

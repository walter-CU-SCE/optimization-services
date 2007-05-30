<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <xsl:template match="/">
        <html>
            <head>
                <link rel="stylesheet"
                    href="http://www.optimiztionservices.org/stylesheets/OSrL.css"/>
            </head>
            <body>
                <h1>Optimization Solution</h1>
                
                <h2>Solver Used</h2>
                <p>
                    <xsl:for-each select="/osrl/resultHeader/serviceName">
                        <b> <xsl:value-of select="text()"/> </b>
                    </xsl:for-each>
                </p>

                <h2>Objective Function Value</h2>
                <p>
                    <xsl:for-each select="/osrl/resultData/optimization/solution/objectives/values/obj">
                       <b> <xsl:value-of select="text()"/> </b>
                    </xsl:for-each>
                </p>

                <h2>Primal Solution</h2>
                <table border="2">
                    <tr>
                        <td>
                            <b>Variable Index</b>
                        </td>
                        <td>
                            <b>Value</b>
                        </td>
                    </tr>
                    <xsl:for-each
                        select="/osrl/resultData/optimization/solution/variables/values/var">
                        <tr>
                            <td>
                                <xsl:value-of select="@idx"/>
                            </td>
                            <td>
                                <xsl:value-of select="text()"/>
                            </td>
                        </tr>
                    </xsl:for-each>
                </table>



                <h2>Dual Solution</h2>
                <table border="2">
                    <tr>
                        <td>
                            <b>Variable Index</b>
                        </td>
                        <td>
                            <b>Value</b>
                        </td>
                    </tr>
                    <xsl:for-each
                        select="/osrl/resultData/optimization/solution/constraints/dualValues/con">
                        <tr>
                            <td>
                                <xsl:value-of select="@idx"/>
                            </td>
                            <td>
                                <xsl:value-of select="text()"/>
                            </td>
                        </tr>
                    </xsl:for-each>
                </table>

            </body>
        </html>
    </xsl:template>
</xsl:stylesheet>

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'ESP32-ThingsBoard-MQTT-Client'
copyright = '2022, Liang Zhuzi'
author = 'Liang Zhuzi'

# The full version, including alpha/beta/rc tags
release = '0.1'
version = '0.1.0'	# //from template

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.duration',		# //from template
    'sphinx.ext.doctest',		# //from template
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',	# //from template
    'sphinx.ext.intersphinx',	# //from template
	'recommonmark',
	'plantweb.directive',
]

# //from template
intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

templates_path = ['_templates']
# //end

exclude_patterns = []

language = 'zh_CN'

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

#html_theme = 'alabaster'
import sphinx_rtd_theme
html_theme = "sphinx_rtd_theme"
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

html_static_path = ['_static']

# -- Options for EPUB output //from template
epub_show_urls = 'footnote'

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Bathymetric Attributed Grid (BAG)'
copyright = '2022, The Open Navigation Surface Project'
author = 'The Open Navigation Surface Project'
release = '2.0.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['sphinx_automodapi.automodapi',
              'sphinx.ext.autodoc',
              'sphinx.ext.autosummary',
              'sphinx.ext.napoleon',
              'sphinx.ext.viewcode',
              'sphinx.ext.intersphinx',
              'breathe',
              'myst_parser']

numpydoc_show_class_members = False

templates_path = ['_templates']
exclude_patterns = ['setup.rst']

# Configuration for autodoc extension
autodoc_member_order = 'alphabetical'
autodoc_class_signature = 'separated'
autoclass_content = 'both'
autodoc_typehints = 'both'
autodoc_default_options = {
    'member-order': 'groupwise',
    'members': True,
    'special-members': '__init__',
    'show-inheritance': True,
}
## Generate autodoc stubs with summaries from code
autosummary_generate = True

intersphinx_mapping = {'python': ('https://docs.python.org/3', None)}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'classic'
html_static_path = ['_static']

# Config for integrating Doxygen C++ docs with Sphinx
breathe_default_project = 'baglib'

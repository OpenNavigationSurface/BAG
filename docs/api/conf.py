import subprocess, os

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

if read_the_docs_build:
  subprocess.call('cd ../..; doxygen docs/api/docs_config.dox', shell=True)

  html_extra_path = ['html']

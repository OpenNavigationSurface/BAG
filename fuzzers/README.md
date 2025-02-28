
# Testing locally
Clone github.com:OpenNavigationSurface/oss-fuzz.git, then run: 
```shell
export PROJECT_NAME=opennavsurf-bag
python infra/helper.py build_image --architecture x86_64 $PROJECT_NAME
python infra/helper.py build_fuzzers --sanitizer address --engine libfuzzer --architecture x86_64 $PROJECT_NAME
```

Note, if running on a non-AMD64 machine (e.g., on an ARM64 M-series macOS), update oss-fuzz's `infra/helper.py` to 
specify the `--platform linux/amd64` option to `docker build`. The diff
will look something like:
```
--- a/infra/helper.py
+++ b/infra/helper.py
@@ -636,6 +636,11 @@ def build_image_impl(project, cache=True, pull=False, architecture='x86_64'):
        'plain',
        '--load',
    ]
  else:
    build_args += [
        '--platform',
        'linux/amd64',
    ]
  if not cache:
    build_args.append('--no-cache')
```

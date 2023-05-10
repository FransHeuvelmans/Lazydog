# lazydog_merge_sqlite

Merge SQLite DB files by copying the tables.
If the tables do not overlap they can be copied straight, if they do overlap there are some checks which see if the tables have the same schema. This isn't bulletproof and is more done on a best-effort basis.

### Creating an executable

After a `pdm install` and `pdm run all` worked, running `python -m nuitka --onefile ../src/lazydog_merge_sqlite/__main__.py` inside the `build` directory should create a single executable.

## TODO
* Support Nuitka - single binary
* Check files if they are sqlite and give better errors based on that
# DataBuildTool

## Install
Inside new Python 3.10 venv.

```{sh}
pip install dbt-sqlite
```

## Usage

Make sure all sources in `transformations/lazydog/models/sources.yml` are defined inside `~/.dbt/profiles.yml` (for each environment).
This should contain all the source data for analytics, visualization, and general processing.
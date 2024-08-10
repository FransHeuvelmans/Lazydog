# Paseo extraction instructions

[Paseo][1] is a free and open source step counting app ([src][2]).

This is a very basic import-tool for sqlite export databases created by the Paseo step-counter.

There are a few basic features:

- Converting an exported database to a typical lazydog-paseo db
- Merge a raw export to a lazydog-paseo-db
- Generate a tsv file based on a time-frame which can be used by vega-lite or gnuplot

## Using `paseo_import`

Transform exported `a.db`.
`paseo_import a.db`

Combine a recent-export (`b.db`) with the previous transformed export.
`paseo_import -i b.db a.db`

Create tsv based on a date-range to get some step-info.
`paseo_import -j "2023-08-13:2023-09-01" ./some_transformed_paseo.db`

Finally check the `paseo_examples/toy_gnu_plot.gp` example on creating a basic visualization of this output.

## Raw SQL code for transforming

To make the data more in line with the rest of Lazydog apply (uses an `sqlite3` client)

```{sh}
sqlite3 user.db < transform_datums.sql
```

[1]: https://f-droid.org/en/packages/ca.chancehorizon.paseo/
[2]: https://gitlab.com/pardomi/paseo/tree/HEAD
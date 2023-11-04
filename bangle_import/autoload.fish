argparse --name=autoload 'h/help' 't/target=' 'd/dir=!test -d "$_flag_value"' -- $argv
if test $status -ne 0
    echo "Error: Invalid arguments"
    return 1
end

# TODO: Needed for sqlite-utils
# THis could be added in a better / more robust way
fish_add_path  ~/miniconda3/envs/lazydog_3_10/bin

for a_dir in (ls -dv $_flag_d/*/)
    echo "Processing $a_dir"
    for bh_file in (ls $a_dir | string match 'Bangle.js*Health*' | sort -V)
        echo "Loading Bangle health file: $bh_file"
        sqlite-utils insert $_flag_t bangle_health $a_dir/$bh_file --csv --detect-types
    end
    for bh_file in (ls $a_dir | string match 'HeartRateRecord*' | sort -V)
        echo "Loading Heartrate file: $bh_file"
        python load_heart.py $a_dir/$bh_file $_flag_t
    end
end
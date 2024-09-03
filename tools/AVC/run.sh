#!/bin/bash

# Set these accordingly
directories=(
    # "../../../../OurSet/CMB-CRC"
    # "../../../../OurSet/CMB-LCA"
    # "../../../../OurSet/CMB-MEL"
    # "../../../../OurSet/CPTAC-LUAD"
    # "../../../../OurSet/CPTAC-PDA"
    # "../../../../OurSet/CPTAC-SAR"
    # "../../../../OurSet/CPTAC-UCEC"
    # "../../../../OurSet/QIN-BREAST"
    "../../../../OurSet/Bruylants"
)
origin=$(pwd)

tools=(
    "lencod"
    "ldecod"
    "lossless.dcfg264e"
)
default="lossless.dcfg264e"

logenc="AVC-enc.log"
logdec="AVC-dec.log"

leftovers=(
    "log.dec"
    "dataDec.txt"
    "stats.dat"
    "log.dat"
    "leakybucketparam.cfg"
    "data.txt"
)

# Copy tools to the target directories
for dir in ${directories[@]}; do
    for tool in ${tools[@]}; do
        if [ ! -f "$dir/$tool" ]; then
            echo "$tool not present in $dir, copying."
            cp "$tool" "$dir/"
        fi
    done
done

# Encode all files
for dir in ${directories[@]}; do
    cd "$dir"
    echo "File,Time" >> "$logenc"
    find . -type f -name "*.cfg264e" | while read -r file; do
        start=$(date +%s.%N)
        ./lencod -d "$default" -f "$file"
        end=$(date +%s.%N)
        elapsed=$(echo "$end - $start" | bc)
        echo "$file,$elapsed" >> "$logenc"
        rm "$file"
    done
    cd "$origin"
done

# Decode all encoded files to check whether the coding was truly lossless
for dir in ${directories[@]}; do
    cd "$dir"
    echo "File,Time" >> "$logdec"
    find . -type f -name "*.cfg264d" | while read -r file; do
        base=${file%.*}
        start=$(date +%s.%N)
        ./ldecod -d "$file"
        end=$(date +%s.%N)
        elapsed=$(echo "$end - $start" | bc)
        echo "$file,$elapsed" >> "$logdec"
        rm "$file"
    done
    cd "$origin"
done
for dir in ${directories[@]}; do
    cd "$dir"
    find . -type f -name "*.264d" | while read -r file; do
        base=${file%.*}
        cmp "$file" "$base.raw"
        rm "$file"
    done
    cd "$origin"
done

# Clean up tools
for dir in ${directories[@]}; do
    for tool in ${tools[@]}; do
        if [ -f "$dir/$tool" ]; then
            rm "$dir/$tool"
        fi
    done
done
for dir in ${directories[@]}; do
    for leftover in ${leftovers[@]}; do
        if [ -f "$dir/$leftover" ]; then
            rm "$dir/$leftover"
        fi
    done
done
for dir in ${directories[@]}; do
    rm "$dir/"*"_rec.raw"
done
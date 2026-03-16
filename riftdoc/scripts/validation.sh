#!/bin/bash
# =============================================================================
# RIFT Documentation Validation Script
# Checks the results of the cleanup migration
# =============================================================================

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== RIFT DOCUMENTATION VALIDATION ===${NC}"
echo

# Check main structure
echo -e "${YELLOW}1. Checking main structure...${NC}"
if [[ -d "docs" ]]; then
    echo "✅ docs/ directory exists"
    
    # Check stages
    stage_count=$(find docs/stages -maxdepth 1 -type d -name "stage-*" | wc -l)
    echo "✅ Found $stage_count stage directories"
    
    # Check toolchain
    if [[ -d "docs/toolchain" ]]; then
        echo "✅ toolchain/ directory exists"
    else
        echo "❌ toolchain/ directory missing"
    fi
    
    # Check other directories
    for dir in build-orchestration compliance specifications examples; do
        if [[ -d "docs/$dir" ]]; then
            echo "✅ $dir/ directory exists"
        else
            echo "❌ $dir/ directory missing"
        fi
    done
else
    echo "❌ docs/ directory not found"
    exit 1
fi

echo

# Check GitBook configuration
echo -e "${YELLOW}2. Checking GitBook configuration...${NC}"
if [[ -f "book.json" ]]; then
    echo "✅ book.json exists"
    if json_pp < book.json > /dev/null 2>&1; then
        echo "✅ book.json is valid JSON"
    else
        echo "❌ book.json has syntax errors"
    fi
else
    echo "❌ book.json missing"
fi

if [[ -f "docs/README.md" ]]; then
    echo "✅ docs/README.md exists"
else
    echo "❌ docs/README.md missing"
fi

if [[ -f "docs/SUMMARY.md" ]]; then
    echo "✅ docs/SUMMARY.md exists"
else
    echo "❌ docs/SUMMARY.md missing"
fi

echo

# Check file migration results
echo -e "${YELLOW}3. Checking file migration results...${NC}"
total_files=$(find docs -type f -name "*.md" -o -name "*.pdf" -o -name "*.txt" | wc -l)
echo "📁 Total documentation files in docs/: $total_files"

# Check each stage for content
for stage in {0..6}; do
    case $stage in
        0) stage_name="stage-0-tokenizer" ;;
        1) stage_name="stage-1-parser" ;;
        2) stage_name="stage-2-ast" ;;
        3) stage_name="stage-3-validator" ;;
        4) stage_name="stage-4-bytecode" ;;
        5) stage_name="stage-5-verifier" ;;
        6) stage_name="stage-6-emitter" ;;
    esac
    
    if [[ -d "docs/stages/$stage_name" ]]; then
        file_count=$(find "docs/stages/$stage_name" -type f | wc -l)
        echo "   Stage $stage ($stage_name): $file_count files"
    fi
done

echo

# Check toolchain content
echo -e "${YELLOW}4. Checking toolchain content...${NC}"
for tool in riftlang-exe shared-objects rift-exe gosilang; do
    if [[ -d "docs/toolchain/$tool" ]]; then
        file_count=$(find "docs/toolchain/$tool" -type f | wc -l)
        echo "   $tool: $file_count files"
    fi
done

echo

# Check git status
echo -e "${YELLOW}5. Checking git status...${NC}"
if git status > /dev/null 2>&1; then
    echo "✅ Git repository initialized"
    
    # Check if there are untracked/uncommitted files
    if [[ -n $(git status --porcelain) ]]; then
        echo "⚠️  There are uncommitted changes"
        echo "   Use 'git status' to see details"
    else
        echo "✅ All changes committed"
    fi
    
    # Check last commit
    last_commit=$(git log -1 --pretty=format:"%h - %s" 2>/dev/null)
    if [[ -n "$last_commit" ]]; then
        echo "✅ Last commit: $last_commit"
    fi
else
    echo "❌ Git repository not properly initialized"
fi

echo

# Check for common issues
echo -e "${YELLOW}6. Checking for common issues...${NC}"

# Check for empty directories
empty_dirs=$(find docs -type d -empty 2>/dev/null)
if [[ -n "$empty_dirs" ]]; then
    echo "⚠️  Found empty directories:"
    echo "$empty_dirs" | sed 's/^/   /'
else
    echo "✅ No empty directories found"
fi

# Check for duplicate files (by name)
echo "🔍 Checking for potential duplicate files..."
find docs -type f -name "*.md" -o -name "*.pdf" | xargs -I {} basename {} | sort | uniq -d | head -5 | while read dup; do
    echo "   Potential duplicate: $dup"
done

echo

# Test GitBook build (if gitbook is available)
echo -e "${YELLOW}7. Testing GitBook compatibility...${NC}"
if command -v gitbook > /dev/null 2>&1; then
    cd docs
    if gitbook build . /tmp/rift-test-build > /dev/null 2>&1; then
        echo "✅ GitBook build successful"
        rm -rf /tmp/rift-test-build
    else
        echo "❌ GitBook build failed"
    fi
    cd ..
else
    echo "⚠️  GitBook CLI not installed - cannot test build"
fi

echo

# Summary
echo -e "${GREEN}=== VALIDATION SUMMARY ===${NC}"
echo "Documentation structure: $(if [[ -d docs ]]; then echo "✅ Created"; else echo "❌ Failed"; fi)"
echo "GitBook configuration: $(if [[ -f book.json && -f docs/README.md && -f docs/SUMMARY.md ]]; then echo "✅ Ready"; else echo "❌ Incomplete"; fi)"
echo "Total migrated files: $total_files"
echo "Git repository: $(if git status > /dev/null 2>&1; then echo "✅ Ready"; else echo "❌ Issues"; fi)"

echo
echo "🚀 Next steps:"
echo "1. Review any issues shown above"
echo "2. Test GitBook: cd docs && gitbook serve"
echo "3. Push to GitHub: git push origin main"
echo "4. Configure GitBook integration online"

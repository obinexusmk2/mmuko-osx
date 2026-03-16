#include <rift_tompgy/trident.h>
#include <memory>

class TridentResolver {
private:
    std::unique_ptr<TridentNode[]> nodes;
    int nodeCount;
    
public:
    // Constructor using C bridge
    TridentResolver(const std::string& spec) {
        nodes.reset(static_cast<TridentNode*>(
            trident_resolve_dependencies(spec.c_str(), STRATEGY_HYBRID)
        ));
    }
    
    // C++-style resolution with A* scoring
    ResolutionResult resolveWithAStar() {
        return applyAStarScoring(nodes.get(), nodeCount);
    }
};
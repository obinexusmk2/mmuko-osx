using MmukoOS.RiftBridge.HumanRights.EvidencePackaging;
using MmukoOS.RiftBridge.HumanRights.GovernanceViewModels;

namespace MmukoOS.RiftBridge.HumanRights;

public interface HumanRightsFirmwareUI
{
    void RenderDiscriminantTimeline(HumanRightsStatusViewModel status);
    void RenderBreachThresholdIndicator(HumanRightsStatusViewModel status);
    void RenderEscalationReadiness(HumanRightsStatusViewModel status);
    void RenderEvidencePackagePreview(EvidencePackagePreview preview);
}

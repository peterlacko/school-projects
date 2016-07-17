<?php

/**
 * Oblibene presenter.
 */
class OblibenePresenter extends BasePresenter
{
    private $oblibeneRepository;
    private $fakultaRepository;

    protected function startup()
    {
	parent::startup();
	if (!$this->getUser()->isLoggedIn()) {
	    $this->flashMessage('Pro spřístupnění toho obsahu se prosím přihlašte.', 'error');
	    $this->redirect('Homepage:');
	}
	$this->oblibeneRepository = $this->context->oblibeneRepository;
	$this->fakultaRepository = $this->context->fakultaRepository;
    }

    public function renderDefault()
    {
	$user_id = $this->getUser()->getIdentity()->user_id;
	$this->template->skoly = $this->oblibeneRepository->findAll()
	    ->where('User_id', $user_id)
	    ->where('ID_fakulty', NULL);
	$fakulty = array();
	foreach ($this->oblibeneRepository->findBy(array('User_id'=>$user_id, 'Nazev_skoly'=>NULL)) as $fakulta) {
	    $fakulta['Nazev_fakulty'] = 
		$this->fakultaRepository->findBy(array('ID_fakulty' => $fakulta->ID_fakulty))->fetch()->Nazev_fakulty;
	    $fakulty[] = $fakulta;
	}
	$this->template->fakulty = $fakulty;
    }

    public function deleteOblibene($id)
    {
		$this->oblibeneRepository->deleteOblibene($id);
		$this->flashMessage('Položka byla odstraněna ze seznamu oblíbených', 'success');
		$this->redirect('Oblibene:');
    }

}

<?php

use Nette\Application\UI\Form;
/*
 * Presenter ktory zaistuje vypis dni otevrenych dveri
 */
class DodPresenter extends BasePresenter
{
    private $dodRepository;
    private $fakultaRepository;
    private $detailFakulty;
    private $record;

    protected function startup()
    {
		parent::startup();
		$this->dodRepository = $this->context->dodRepository;
		$this->fakultaRepository = $this->context->fakultaRepository;
    }

    public function actionAdddod($id) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Dod:');
	}
	$this->detailFakulty = $this->fakultaRepository->findBy(array('ID_fakulty' => $id))->fetch();
	if ($this->detailFakulty === FALSE) {
		$this->setView('notFound');
	}
    }

    public function actionEditdod($id_fakulty, $id_dod) {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Dod:');
	}
	$this->detailFakulty = $this->fakultaRepository->findBy(array('ID_fakulty' => $id_fakulty))->fetch();
	if ($this->detailFakulty === FALSE) {
		$this->setView('notFound');
	}
	$this->record= $this->dodRepository->findBy(array('ID_DOD' => $id_dod))->fetch();
	if (!$this->record) {
	    $this->setView('notFound');
	}
	$this['editDodForm']->setDefaults(array(
	    'zacatek' => $this->record['Zacatek'],
	    'konec' => $this->record['Konec'],
	    'podrobnosti' => $this->record['Podrobnosti']
	));
    }

    public function renderDefault()
    {
	$dods = $this->dodRepository->showDODs();
	$dods2 = array();
	foreach ($dods as $dod) {
	    $dod['Nazev_fakulty'] = $this->fakultaRepository->findAll()->get($dod['ID_fakulty'])->Nazev_fakulty;
	    array_push($dods2, $dod);
	}
	$this->template->tasks = $dods2;
    }
	
    public function renderAdddod()
    {
	$this->template->detailFakulty = $this->detailFakulty;
    }

    public function renderEditdod()
    {
	$this->template->detailFakulty = $this->detailFakulty;
    }

    /*
     * pridani DOD
     */
    protected function createComponentAddDodForm()
    {
	$form = new Form;
	$now = new DateTime();
	$form->addText('zacatek', 'Začátek:')
	    ->setRequired('Zadejte prosím datum a čas konání DOD')
		->setValue($now->format('Y-m-d H-i-s'));
	$form->addText('konec', 'Konec:')
		->setRequired('Zadejte prosím datum a čas konce DOD')
		->setValue($now->modify( '+6 hour' )->format('Y-m-d H-i-s'));
	$form->addTextArea('podrobnosti', 'Podrobnosti:')
	    ->addRule(Form::MAX_LENGTH, 'Podrobnosti jsou omezena na 200 znaku', 200);
	$form->addSubmit('addDod', 'Přidat')
	    ->onClick[] = $this->addDodFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'addDodFormCancel');
	return $form;
    }

    public function addDodFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values=$btn->form->getValues();
	try {
	    $this->dodRepository->addDod($values, $this->detailFakulty['ID_fakulty']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	}
    }

    /*
     * editace dod
     */
    protected function createComponentEditDodForm()
    {
	$form = new Form;
	$form->addText('zacatek', 'Začátek:')
	    ->setRequired('Zadejte prosím datum a čas konání DOD');
	$form->addText('konec', 'Konec:');
	$form->addTextArea('podrobnosti', 'Podrobnosti:')
	    ->addRule(Form::MAX_LENGTH, 'Podrobnosti jsou omezena na 200 znaku', 200);
	$form->addSubmit('addDod', 'Upravit')
	    ->onClick[] = $this->editDodFormSubmitted;
	$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'addDodFormCancel');
	return $form;
    }
    public function editDodFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values = $btn->form->getValues();
	try {
	    $this->dodRepository->getconnection()->exec(
		'UPDATE dod SET Zacatek=?, Konec=?, Podrobnosti=? WHERE ID_DOD=?',
		$values->zacatek, $values->konec, $values->podrobnosti, $this->record['ID_DOD']);
	    $this->flashMessage('Operace byla úspěšně provedena', 'success');
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	} catch (Exception $e) {
	    if (!($e instanceof Nette\Application\AbortException)) {
		$this->flashMessage('Operaci nebylo možné provést', 'error');
	    }
	    $this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
	}
    }

    public function addDodFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
	$this->redirect('Fakulta:detail', $this->detailFakulty['ID_fakulty']);
    }
	
    /*
     * odstraneni DOD
     */
    public function deleteDod($id)
    {
		$this->dodRepository->deleteDod($id);
		$this->flashMessage('DOD byl úspěšně odstraněn.', 'success');
		/*$this->redirect('DOD:');*/
    }
}
